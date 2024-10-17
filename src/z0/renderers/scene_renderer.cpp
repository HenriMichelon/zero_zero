module;
#include <cassert>
#include <json.hpp>
#include <volk.h>
#include "stb_image_write.h"
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :ModelsRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;
import :Node;
import :MeshInstance;
import :ShadowMapRenderer;
import :SkyboxRenderer;
import :Environment;
import :Material;
import :Resource;
import :Buffer;
import :Shader;
import :Image;
import :Light;
import :DirectionalLight;
import :OmniLight;
import :SpotLight;
import :FrameBuffer;
import :ShadowMapFrameBuffer;
import :ColorFrameBuffer;
import :DepthFrameBuffer;
import :Skybox;
import :SceneRenderer;
import :FrustumCulling;

namespace z0 {

    void sc_stb_write_func(void *context, void *data, const int size) {
        auto *buffer = static_cast<vector<unsigned char> *>(context);
        auto *ptr    = static_cast<unsigned char *>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    SceneRenderer::SceneRenderer(Device &device, const string &shaderDirectory, const vec3 clearColor) :
        ModelsRenderer{device, shaderDirectory, clearColor},
        colorFrameBufferMultisampled{device, true},
        materialsIndicesAllocation(MAX_MATERIALS, Resource::INVALID_ID) {
        createImagesResources();
        OutlineMaterials::_initialize();
        createOrUpdateResources(true, &pushConstantRange);
    }

    void SceneRenderer::cleanup() {
        materialShaders.clear();
        materialsBuffers.clear();
        shadowMapsUniformBuffers.clear();
        pointLightUniformBuffers.clear();
        if (blankImage != nullptr) {
            blankImage.reset();
            blankImageData.clear();
        }
        if (skyboxRenderer != nullptr)
            skyboxRenderer->cleanup();
        for (const auto &shadowMapRenderer : shadowMapRenderers) {
            device.unRegisterRenderer(shadowMapRenderer);
            shadowMapRenderer->cleanup();
        }
        shadowMapRenderers.clear();
        opaquesModels.clear();
        omniLights.clear();
        ModelsRenderer::cleanup();
    }

    void SceneRenderer::addNode(const shared_ptr<Node> &node) {
        if (auto *skybox = dynamic_cast<Skybox *>(node.get())) {
            skyboxRenderer = make_unique<SkyboxRenderer>(device, shaderDirectory, clearColor);
            skyboxRenderer->loadScene(skybox->getCubemap());
            return;
        }
        if (currentEnvironment == nullptr) {
            if (auto *environment = dynamic_cast<Environment *>(node.get())) {
                currentEnvironment = environment;
                // log("Using environment", environment->toString());
                return;
            }
        }
        if (directionalLight == nullptr) {
            if (auto *light = dynamic_cast<DirectionalLight *>(node.get())) {
                directionalLight = light;
                // log("Using directional light", directionalLight->toString());
                enableLightShadowCasting(directionalLight);
                return;
            }
        }
        if (auto *omniLight = dynamic_cast<OmniLight *>(node.get())) {
            omniLights.push_back(omniLight);
            if (const auto *spotLight = dynamic_cast<SpotLight *>(omniLight)) {
                enableLightShadowCasting(spotLight);
            }
        }
        ModelsRenderer::addNode(node);
    }

    void SceneRenderer::removeNode(const shared_ptr<Node> &node) {
        if (dynamic_cast<Skybox *>(node.get())) {
            skyboxRenderer.reset();
        } else if (const auto *environment = dynamic_cast<Environment *>(node.get())) {
            if (currentEnvironment == environment) {
                currentEnvironment = nullptr;
            }
        } else if (const auto *light = dynamic_cast<DirectionalLight *>(node.get())) {
            if (directionalLight == light) {
                disableLightShadowCasting(directionalLight);
                directionalLight = nullptr;
            }
        } else if (const auto *omniLight = dynamic_cast<OmniLight *>(node.get())) {
            disableLightShadowCasting(omniLight);
        } else {
            ModelsRenderer::removeNode(node);
        }
    }

    void SceneRenderer::preUpdateScene() {
        for (const auto &material : OutlineMaterials::_all()) {
            if (find(materials.begin(), materials.end(), material.get()) != materials.end())
                continue;
            material->_incrementReferenceCounter();
            materialsIndices[material->getId()] = materials.size();
            materials.push_back(material.get());
            descriptorSetNeedUpdate = true;
        }
    }

    void SceneRenderer::postUpdateScene() {
        createOrUpdateResources(true, &pushConstantRange);
        for (const auto &material : OutlineMaterials::_all()) {
            loadShadersMaterials(material.get());
        }
        for (const auto &renderer : shadowMapRenderers) {
            renderer->loadScene(models);
        }
    }

    void SceneRenderer::addingModel(MeshInstance *meshInstance, const uint32_t modelIndex) {
        opaquesModels.push_back(meshInstance);
        modelsIndices[meshInstance->getId()] = modelIndex;
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            material->_incrementReferenceCounter();
            if (find(materials.begin(), materials.end(), material.get()) != materials.end())
                continue;
            if (materials.size() >= MAX_MATERIALS)
                die("Maximum number of materials reached");
            // We have a new material, allocate the first free index
            const auto it = std::find_if(materialsIndicesAllocation.begin(), materialsIndicesAllocation.end(),
                [&](const Resource::id_t &id) {
                    return id == Resource::INVALID_ID;
                });
            assert(it != materialsIndicesAllocation.end());
            const auto index = it - materialsIndicesAllocation.begin();
            materialsIndices[material->getId()] = index;
            materialsIndicesAllocation[index] = material->getId();
            materials.push_back(material.get());
            // Force material data to be written to GPU memory
            material->_setDirty();
            // Load textures for standards materials
            if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                if (standardMaterial->getAlbedoTexture() != nullptr)
                    addImage(standardMaterial->getAlbedoTexture()->getImage());
                if (standardMaterial->getSpecularTexture() != nullptr)
                    addImage(standardMaterial->getSpecularTexture()->getImage());
                if (standardMaterial->getNormalTexture() != nullptr)
                    addImage(standardMaterial->getNormalTexture()->getImage());
            }
        }
    }

    void SceneRenderer::addedModel(MeshInstance *meshInstance) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            if (const auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material.get())) {
                loadShadersMaterials(shaderMaterial);
            }
        }
        opaquesModels.sort([](const MeshInstance *a, const MeshInstance*b) { return *a < *b; });
    }

    void SceneRenderer::removingModel(MeshInstance *meshInstance) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            if (find(materials.begin(), materials.end(), material.get()) != materials.end()) {
                if (material->_decrementReferenceCounter()) {
                    if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                        if (standardMaterial->getAlbedoTexture() != nullptr)
                            removeImage(standardMaterial->getAlbedoTexture()->getImage());
                        if (standardMaterial->getSpecularTexture() != nullptr)
                            removeImage(standardMaterial->getSpecularTexture()->getImage());
                        if (standardMaterial->getNormalTexture() != nullptr)
                            removeImage(standardMaterial->getNormalTexture()->getImage());
                    } else if (const auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material.get())) {
                        const auto &shader = materialShaders[shaderMaterial->getFragFileName()];
                        if (shader->_decrementReferenceCounter()) {
                            materialShaders.erase(shaderMaterial->getFragFileName());
                        }
                    }
                    materials.remove(material.get());
                    // Free the material index
                    materialsIndicesAllocation[materialsIndices[material->getId()]] = Resource::INVALID_ID;
                }
            }
        }

        const auto it = find(opaquesModels.begin(), opaquesModels.end(), meshInstance);
        if (it != opaquesModels.end()) {
            opaquesModels.erase(it);
            modelsIndices.erase(meshInstance->getId());
        }
    }

    void SceneRenderer::loadShadersMaterials(const ShaderMaterial *material) {
        if (!materialShaders.contains(material->getFragFileName())) {
            if (!material->getVertFileName().empty()) {
                materialShaders[material->getVertFileName()] = createShader(
                        material->getVertFileName(), VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
                materialShaders[material->getVertFileName()]->_incrementReferenceCounter();
            }
            if (!material->getFragFileName().empty()) {
                materialShaders[material->getFragFileName()] =
                        createShader(material->getFragFileName(), VK_SHADER_STAGE_FRAGMENT_BIT, 0);
                materialShaders[material->getFragFileName()]->_incrementReferenceCounter();
            }
        }
    }

    void SceneRenderer::activateCamera(Camera *camera) {
        ModelsRenderer::activateCamera(camera);
        for (const auto &renderer : shadowMapRenderers) {
            renderer->activateCamera(camera);
        }
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        if (currentCamera == nullptr)
            return;
        if (skyboxRenderer != nullptr)
            skyboxRenderer->update(currentCamera, currentEnvironment, currentFrame);
        if (models.empty())
            return;

        GobalUniformBuffer globalUbo{
                .projection           = currentCamera->getProjection(),
                .view                 = currentCamera->getView(),
                .cameraPosition       = currentCamera->getPositionGlobal(),
                .haveDirectionalLight = directionalLight != nullptr,
                .pointLightsCount     = static_cast<uint32_t>(omniLights.size()),
                .shadowMapsCount      = static_cast<uint32_t>(shadowMapRenderers.size()),
        };
        if (globalUbo.haveDirectionalLight) {
            globalUbo.directionalLight = {
                    .direction = directionalLight->getFrontVector(),
                    .color    = directionalLight->getColorAndIntensity(),
                    .specular = directionalLight->getSpecularIntensity(),
            };
        }
        if (currentEnvironment != nullptr) {
            globalUbo.ambient = currentEnvironment->getAmbientColorAndIntensity();
        }

        if (enableShadowMapRenders && (globalUbo.shadowMapsCount > 0)) {
            auto shadowMapArray = make_unique<ShadowMapUniformBuffer[]>(globalUbo.shadowMapsCount);
            for (uint32_t i = 0; i < globalUbo.shadowMapsCount; i++) {
                if ((globalUbo.cascadedShadowMapIndex == -1) && shadowMapRenderers[i]->isCascaded()) {
                    // Activate the first cascaded shadow map found
                    globalUbo.cascadedShadowMapIndex = i;
                    for (int cascadeIndex = 0; cascadeIndex < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; cascadeIndex++) {
                        shadowMapArray[i].lightSpace[cascadeIndex] = shadowMapRenderers[i]->getLightSpace(cascadeIndex);
                        shadowMapArray[i].cascadeSplitDepth[cascadeIndex] = shadowMapRenderers[i]->getCascadeSplitDepth(cascadeIndex);
                    }
                } else {
                    // Just copy the light space matrix for non cascaded shadow maps
                    shadowMapArray[i].lightSpace[0] = shadowMapRenderers[i]->getLightSpace(0);
                }
            }
            writeUniformBuffer(shadowMapsUniformBuffers, currentFrame, shadowMapArray.get());
        }
        writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo); // after the shadows maps for .cascadedShadowMapIndex

        if (globalUbo.pointLightsCount > 0) {
            auto pointLightsArray = make_unique<PointLightUniformBuffer[]>(globalUbo.pointLightsCount);
            for (uint32_t i = 0; i < globalUbo.pointLightsCount; i++) {
                auto *omniLight               = omniLights[i];
                pointLightsArray[i].position  = omniLight->getPositionGlobal();
                pointLightsArray[i].color     = omniLight->getColorAndIntensity();
                pointLightsArray[i].specular  = omniLight->getSpecularIntensity();
                pointLightsArray[i].constant  = omniLight->getAttenuation();
                pointLightsArray[i].linear    = omniLight->getLinear();
                pointLightsArray[i].quadratic = omniLight->getQuadratic();
                if (auto *spot = dynamic_cast<SpotLight *>(omniLight)) {
                    pointLightsArray[i].isSpot    = true;
                    pointLightsArray[i].direction = normalize(mat3{spot->getTransformGlobal()} * AXIS_FRONT);
                    pointLightsArray[i].cutOff    = spot->getCutOff();
                    pointLightsArray[i].outerCutOff = spot->getOuterCutOff();
                }
            }
            writeUniformBuffer(pointLightUniformBuffers, currentFrame, pointLightsArray.get());
        }

        uint32_t modelIndex = 0;
        auto modelUBOArray = make_unique<ModelUniformBuffer[]>(models.size());
        for (const auto &meshInstance : models) {
            modelUBOArray[modelIndex].matrix = meshInstance->getTransformGlobal();
            writeUniformBuffer(modelUniformBuffers, currentFrame, modelUBOArray.get());
            modelIndex += 1;
        }

        for (auto *material : materials) {
            if (material->_isDirty()) {
                auto materialBuffer = MaterialUniformBuffer{};
                materialBuffer.transparency = material->getTransparency();
                materialBuffer.alphaScissor = material->getAlphaScissor();
                if (auto *standardMaterial = dynamic_cast<StandardMaterial *>(material)) {
                    materialBuffer.albedoColor = standardMaterial->getAlbedoColor().color;
                    if (standardMaterial->getAlbedoTexture() != nullptr) {
                        materialBuffer.diffuseIndex = imagesIndices[standardMaterial->getAlbedoTexture()->getImage()->getId()];
                        const auto &transform    = standardMaterial->getTextureTransform();
                        if (transform != nullptr) {
                            materialBuffer.hasTransform  = true;
                            materialBuffer.textureOffset = transform->offset;
                            materialBuffer.textureScale  = transform->scale;
                        }
                    }
                    if (standardMaterial->getSpecularTexture() != nullptr) {
                        materialBuffer.specularIndex =
                                imagesIndices[standardMaterial->getSpecularTexture()->getImage()->getId()];
                    }
                    if (standardMaterial->getNormalTexture() != nullptr) {
                        materialBuffer.normalIndex = imagesIndices[standardMaterial->getNormalTexture()->getImage()->getId()];
                    }
                } else if (auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material)) {
                    for (int i = 0; i < ShaderMaterial::MAX_PARAMETERS; i++) {
                        materialBuffer.parameters[i] = shaderMaterial->getParameter(i);
                    }
                }
                material->_clearDirty();
                materialsBuffers[currentFrame]->writeToBuffer(
                    &materialBuffer,
                    materialBufferSize,
                    materialBufferSize * materialsIndices[material->getId()]);
            }
        }
    }

    void SceneRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        if (currentCamera == nullptr)
            return;
        setInitialState(commandBuffer);
        if (!models.empty()) {
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            bindDescriptorSets(commandBuffer, currentFrame);
            drawModels(commandBuffer, currentFrame, opaquesModels);
        }
        if (skyboxRenderer != nullptr)
            skyboxRenderer->recordCommands(commandBuffer, currentFrame);
    }

    void SceneRenderer::createDescriptorSetLayout() {
        descriptorPool =
                DescriptorPool::Builder(device)
                        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                        // global UBO
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                        // models UBO, one array
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                        // materials UBO, one array
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                        // images textures
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                        // shadow maps UBO, one array
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                        // shadow maps
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                        // pointlightarray UBO, one array
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                        .build();

        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0,
                                        // global UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_ALL_GRAPHICS)
                            .addBinding(1,
                                        // models UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_VERTEX_BIT)
                            .addBinding(2,
                                        // materials UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_ALL_GRAPHICS)
                            .addBinding(3,
                                        // images textures
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        MAX_IMAGES)
                            .addBinding(4,
                                        // shadow maps array UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(5,
                                        // shadow maps
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        MAX_SHADOW_MAPS)
                            .addBinding(6,
                                        // PointLight array UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .build();

        // Create an in-memory default blank image
        // and initialize the image info array with this image
        if (blankImage == nullptr) {
            const auto data = new unsigned char[1 * 1 * 3];
            data[0]         = 0;
            data[1]         = 0;
            data[2]         = 0;
            stbi_write_jpg_to_func(sc_stb_write_func, &blankImageData, 1, 1, 3, data, 100);
            delete[] data;
            blankImage = make_unique<Image>(device, "Blank", 1, 1, blankImageData.size(), blankImageData.data());
        }

        globalUniformBufferSize = sizeof(GobalUniformBuffer);
        createUniformBuffers(globalUniformBuffers, globalUniformBufferSize);
    }

    void SceneRenderer::createOrUpdateDescriptorSet(const bool create) {
        if (!models.empty() && (modelUniformBufferCount != models.size())) {
            modelUniformBufferCount = models.size();
            createUniformBuffers(modelUniformBuffers, modelUniformBufferSize * modelUniformBufferCount);
        }
        if (modelUniformBufferCount == 0) {
            modelUniformBufferCount = 1;
            createUniformBuffers(modelUniformBuffers, modelUniformBufferSize);
        }
        if (materialsBuffers[0] == nullptr) {
            createUniformBuffers(materialsBuffers, materialBufferSize * MAX_MATERIALS);
        }

        if ((!shadowMapRenderers.empty()) && (shadowMapUniformBufferCount != shadowMapRenderers.size())) {
            shadowMapUniformBufferCount = shadowMapRenderers.size();
            createUniformBuffers(shadowMapsUniformBuffers, shadowMapUniformBufferSize * shadowMapUniformBufferCount);
        }
        if (shadowMapUniformBufferCount == 0) {
            shadowMapUniformBufferCount = 1;
            createUniformBuffers(shadowMapsUniformBuffers, shadowMapUniformBufferSize);
        }
        if ((!omniLights.empty()) && (pointLightUniformBufferCount != omniLights.size())) {
            pointLightUniformBufferCount = omniLights.size();
            createUniformBuffers(pointLightUniformBuffers, pointLightUniformBufferSize * pointLightUniformBufferCount);
        }
        if (pointLightUniformBufferCount == 0) {
            pointLightUniformBufferCount = 1;
            createUniformBuffers(pointLightUniformBuffers, pointLightUniformBufferSize);
        }

        uint32_t imageIndex = 0;
        for (const auto &image : images) {
            imagesInfo[imageIndex] = image->_getImageInfo();
            imageIndex += 1;
        }
        // initialize the rest of the image info array with the blank image
        for (uint32_t i = imageIndex; i < imagesInfo.size(); i++) {
            imagesInfo[i] = blankImage->_getImageInfo();
        }

        imageIndex = 0;
        for (const auto &shadowMapRenderer : shadowMapRenderers) {
            const auto &shadowMap      = shadowMapRenderer->getShadowMap();
            shadowMapsInfo[imageIndex] = VkDescriptorImageInfo{
                    .sampler     = shadowMap->getSampler(),
                    .imageView   = shadowMap->FrameBuffer::getImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            };
            imageIndex += 1;
        }
        // initialize the rest of the shadow map info array with the blank image
        for (uint32_t i = imageIndex; i < shadowMapsInfo.size(); i++) {
            shadowMapsInfo[i] = blankImage->_getImageInfo();
        }

        for (uint32_t i = 0; i < descriptorSet.size(); i++) {
            auto globalBufferInfo     = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto modelBufferInfo      = modelUniformBuffers[i]->descriptorInfo(modelUniformBufferSize *
                                                                                modelUniformBufferCount);
            auto materialBufferInfo   = materialsBuffers[i]->descriptorInfo(materialBufferSize * MAX_MATERIALS);
            auto shadowMapBufferInfo  = shadowMapsUniformBuffers[i]->descriptorInfo(shadowMapUniformBufferSize *
                                                                                   shadowMapUniformBufferCount);
            auto pointLightBufferInfo = pointLightUniformBuffers[i]->descriptorInfo(pointLightUniformBufferSize *
                                                                                    pointLightUniformBufferCount);
            auto writer               = DescriptorWriter(*setLayout, *descriptorPool)
                                  .writeBuffer(0, &globalBufferInfo)
                                  .writeBuffer(1, &modelBufferInfo)
                                  .writeBuffer(2, &materialBufferInfo)
                                  .writeImage(3, imagesInfo.data())
                                  .writeBuffer(4, &shadowMapBufferInfo)
                                  .writeImage(5, shadowMapsInfo.data())
                                  .writeBuffer(6, &pointLightBufferInfo);
            if (create) {
                if (!writer.build(descriptorSet[i]))
                    die("Cannot allocate descriptor set");
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void SceneRenderer::loadShaders() {
        if (skyboxRenderer != nullptr) {
            skyboxRenderer->loadShaders();
        }
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::createImagesResources() {
        colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device);
        if (depthFrameBuffer == nullptr) {
            depthFrameBuffer         = make_shared<DepthFrameBuffer>(device, true);
            resolvedDepthFrameBuffer = make_shared<DepthFrameBuffer>(device, false);
        } else {
            depthFrameBuffer->createImagesResources();
        }
    }

    void SceneRenderer::cleanupImagesResources() {
        if (depthFrameBuffer != nullptr) {
            resolvedDepthFrameBuffer->cleanupImagesResources();
        }
        colorFrameBufferHdr->cleanupImagesResources();
        colorFrameBufferMultisampled.cleanupImagesResources();
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        colorFrameBufferHdr->createImagesResources();
        colorFrameBufferMultisampled.createImagesResources();
        if (depthFrameBuffer != nullptr) {
            resolvedDepthFrameBuffer->createImagesResources();
        }
    }

    void SceneRenderer::beginRendering(const VkCommandBuffer commandBuffer) {
        // https://lesleylai.info/en/vk-khr-dynamic-rendering/
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferMultisampled.getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        // Color attachment : where the rendering is done (multi sampled memory image)
        // Resolved into a non-multi sampled image
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView          = colorFrameBufferMultisampled.getImageView(),
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT,
                .resolveImageView   = colorFrameBufferHdr->getImageView(),
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue         = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView          = depthFrameBuffer->getImageView(),
                .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT,
                .resolveImageView   = resolvedDepthFrameBuffer->getImageView(),
                .resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue         = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                                            .pNext                = nullptr,
                                            .renderArea           = {{0, 0}, device.getSwapChainExtent()},
                                            .layerCount           = 1,
                                            .colorAttachmentCount = 1,
                                            .pColorAttachments    = &colorAttachmentInfo,
                                            .pDepthAttachment     = &depthAttachmentInfo,
                                            .pStencilAttachment   = nullptr};
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void SceneRenderer::endRendering(const VkCommandBuffer commandBuffer, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                             : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      0,
                                      isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer,
                                      resolvedDepthFrameBuffer->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                                      0,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void SceneRenderer::addImage(const shared_ptr<Image> &image) {
        image->_incrementReferenceCounter();
        if (find(images.begin(), images.end(), image.get()) != images.end())
            return;
        if (images.size() == MAX_IMAGES)
            die("Maximum images count reached for the scene renderer");
        imagesIndices[image->getId()] = static_cast<int32_t>(images.size());
        images.push_back(image.get());
    }

    void SceneRenderer::removeImage(const shared_ptr<Image> &image) {
        if (find(images.begin(), images.end(), image.get()) != images.end()) {
            if (image->_decrementReferenceCounter()) {
                images.remove(image.get());
                // Rebuild the image index
                imagesIndices.clear();
                uint32_t imageIndex = 0;
                for (const auto &img : images) {
                    imagesIndices[img->getId()] = static_cast<int32_t>(imageIndex);
                    imageIndex += 1;
                }
            }
        }
    }

    void SceneRenderer::drawModels(const VkCommandBuffer commandBuffer, const uint32_t currentFrame,
                                   const list<MeshInstance *> &modelsToDraw) {
        auto shadersChanged = false;
        const auto cameraFrustum = Frustum{
            currentCamera,
            currentCamera->getFov(),
            currentCamera->getNearClipDistance(),
            currentCamera->getFarClipDistance()
        };

        // Used to reduce vkCmdSetCullMode calls
        auto lastCullMode = CULLMODE_BACK;
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);

        // Used to reduce vkCmdBindVertexBuffers & vkCmdBindIndexBuffer calls
        auto lastMeshId = Resource::id_t{numeric_limits<uint32_t>::max()};

        for (const auto &meshInstance : modelsToDraw) {
            if (meshInstance->isValid() && cameraFrustum.isOnFrustum(meshInstance)) {
                const auto &modelIndex = modelsIndices[meshInstance->getId()];
                const auto &model      = meshInstance->getMesh();
                for (const auto &surface : model->getSurfaces()) {
                    if (meshInstance->isOutlined()) {
                        const auto        &material      = meshInstance->getOutlineMaterial();
                        const auto        &materialIndex = materialsIndices[material->getId()];
                        auto pushConstants = PushConstants {
                            .modelIndex = static_cast<int>(modelIndex),
                            .materialIndex = static_cast<int>(materialIndex)
                        };
                        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS,
                                            0, sizeof(PushConstants), &pushConstants);
                        vkCmdBindShadersEXT(commandBuffer,
                                            1,
                                            materialShaders[material->getVertFileName()]->getStage(),
                                            materialShaders["outline.vert"]->getShader());
                        vkCmdBindShadersEXT(commandBuffer,
                                            1,
                                            materialShaders[material->getFragFileName()]->getStage(),
                                            materialShaders["outline.frag"]->getShader());
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
                        if (lastMeshId == model->getId()) {
                            model->_bindlessDraw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                        } else {
                            model->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                            lastMeshId = model->getId();
                        }
                        vkCmdSetCullMode(commandBuffer,
                                        lastCullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE
                                                : lastCullMode == CULLMODE_BACK
                                                ? VK_CULL_MODE_BACK_BIT
                                                : VK_CULL_MODE_FRONT_BIT);
                        vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
                        vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
                    }

                    if (const auto shaderMaterial = dynamic_cast<ShaderMaterial *>(surface->material.get())) {
                        if (!shaderMaterial->getFragFileName().empty()) {
                            Shader *material = materialShaders[shaderMaterial->getFragFileName()].get();
                            vkCmdBindShadersEXT(commandBuffer, 1, material->getStage(), material->getShader());
                            shadersChanged = true;
                        }
                        if (!shaderMaterial->getVertFileName().empty()) {
                            Shader *material = materialShaders[shaderMaterial->getVertFileName()].get();
                            vkCmdBindShadersEXT(commandBuffer, 1, material->getStage(), material->getShader());
                            shadersChanged = true;
                        }
                    }
                    auto cullMode = surface->material->getCullMode();
                    if (cullMode != lastCullMode) {
                        vkCmdSetCullMode(commandBuffer,
                                         cullMode == CULLMODE_DISABLED ? VK_CULL_MODE_NONE
                                                 : cullMode == CULLMODE_BACK
                                                 ? VK_CULL_MODE_BACK_BIT
                                                 : VK_CULL_MODE_FRONT_BIT);
                        lastCullMode = cullMode;
                    }
                    auto pushConstants = PushConstants {
                        .modelIndex = static_cast<int>(modelIndex),
                        .materialIndex = static_cast<int>(materialsIndices[surface->material->getId()])
                    };
                    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS,
                                        0, sizeof(PushConstants), &pushConstants);
                    if (lastMeshId == model->getId()) {
                        model->_bindlessDraw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    } else {
                        model->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                        lastMeshId = model->getId();
                    }
                    if (shadersChanged) {
                        vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
                        vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
                        shadersChanged = false;
                    }
                }
            }
        }
    }

    [[nodiscard]] shared_ptr<ShadowMapRenderer> SceneRenderer::findShadowMapRenderer(const Light *light) const {
        const auto it =
                std::find_if(shadowMapRenderers.begin(),
                             shadowMapRenderers.end(),
                             [light](const shared_ptr<ShadowMapRenderer> &e) { return e->getLight() == light; });
        return (it == shadowMapRenderers.end() ? nullptr : *it);
    }

    void SceneRenderer::enableLightShadowCasting(const Light *light) {
        if (enableShadowMapRenders && (light->getCastShadows() && (shadowMapRenderers.size() < MAX_SHADOW_MAPS))) {
            const auto shadowMapRenderer = make_shared<ShadowMapRenderer>(device, shaderDirectory, light);
            shadowMapRenderer->activateCamera(currentCamera);
            shadowMapRenderers.push_back(shadowMapRenderer);
            device.registerRenderer(shadowMapRenderer);
        }
    }

    void SceneRenderer::disableLightShadowCasting(const Light *light) {
        if (enableShadowMapRenders) {
            if (const auto renderer = findShadowMapRenderer(light); renderer != nullptr) {
                device.unRegisterRenderer(renderer);
                erase(shadowMapRenderers, renderer);
            }
        }
    }

} // namespace z0
