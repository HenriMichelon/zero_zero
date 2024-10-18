module;
#include <cassert>
#include <glm/detail/type_mat3x4.hpp>
#include <json.hpp>
#include <volk.h>

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
import :SampledFrameBuffer;

namespace z0 {

    SceneRenderer::SceneRenderer(Device &device, const string &shaderDirectory, const vec3 clearColor) :
        ModelsRenderer{device, shaderDirectory, clearColor} {
        createImagesResources();
        OutlineMaterials::_initialize();
        for(uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frameData[i].colorFrameBufferMultisampled = make_unique<ColorFrameBuffer>(device, true);
            frameData[i].materialsIndicesAllocation = vector<Resource::id_t>(MAX_MATERIALS, Resource::INVALID_ID);
        }
        createOrUpdateResources(true, &pushConstantRange);
    }

    void SceneRenderer::cleanup() {
        blankImage.reset();
        for(auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            frameData[i].materialShaders.clear();
            frameData[i].materialsBuffer.reset();
            frameData[i].shadowMapsUniformBuffer.reset();
            frameData[i].pointLightUniformBuffer.reset();
            if (frameData[i].skyboxRenderer != nullptr)
                frameData[i].skyboxRenderer->cleanup();
            frameData[i].opaquesModels.clear();
            frameData[i].omniLights.clear();
        }
        for (const auto &shadowMapRenderer : shadowMapRenderers) {
            device.unRegisterRenderer(shadowMapRenderer);
            shadowMapRenderer->cleanup();
        }
        shadowMapRenderers.clear();
        ModelsRenderer::cleanup();
    }

    void SceneRenderer::addNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (auto *skybox = dynamic_cast<Skybox *>(node.get())) {
            frameData[currentFrame].skyboxRenderer = make_unique<SkyboxRenderer>(device, shaderDirectory, clearColor);
            frameData[currentFrame].skyboxRenderer->loadScene(skybox->getCubemap());
            return;
        }
        if (frameData[currentFrame].currentEnvironment == nullptr) {
            if (auto *environment = dynamic_cast<Environment *>(node.get())) {
                frameData[currentFrame].currentEnvironment = environment;
                return;
            }
        }
        if (frameData[currentFrame].directionalLight == nullptr) {
            if (auto *light = dynamic_cast<DirectionalLight *>(node.get())) {
                frameData[currentFrame].directionalLight = light;
                enableLightShadowCasting(frameData[currentFrame].directionalLight);
                return;
            }
        }
        if (auto *omniLight = dynamic_cast<OmniLight *>(node.get())) {
            frameData[currentFrame].omniLights.push_back(omniLight);
            if (const auto *spotLight = dynamic_cast<SpotLight *>(omniLight)) {
                enableLightShadowCasting(spotLight);
            }
        }
        ModelsRenderer::addNode(node, currentFrame);
    }

    void SceneRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (dynamic_cast<Skybox *>(node.get())) {
            frameData[currentFrame].skyboxRenderer.reset();
        } else if (const auto *environment = dynamic_cast<Environment *>(node.get())) {
            if (frameData[currentFrame].currentEnvironment == environment) {
                frameData[currentFrame].currentEnvironment = nullptr;
            }
        } else if (const auto *light = dynamic_cast<DirectionalLight *>(node.get())) {
            if (frameData[currentFrame].directionalLight == light) {
                disableLightShadowCasting(frameData[currentFrame].directionalLight);
                frameData[currentFrame].directionalLight = nullptr;
            }
        } else if (const auto *omniLight = dynamic_cast<OmniLight *>(node.get())) {
            disableLightShadowCasting(omniLight);
            frameData[currentFrame].omniLights.remove(omniLight);
        } else {
            ModelsRenderer::removeNode(node, currentFrame);
        }
    }

    void SceneRenderer::preUpdateScene(const uint32_t currentFrame) {
        for (const auto &material : OutlineMaterials::_all()) {
            if (!frameData[currentFrame].materialsIndices.contains(material->getId())) {
                material->_incrementReferenceCounter();
                addMaterial(material, currentFrame);
                descriptorSetNeedUpdate = true;
            }
        }
    }

    void SceneRenderer::addMaterial(const shared_ptr<Material> &material, const uint32_t currentFrame) {
        // We have a new material, allocate the first free buffer index
        const auto it = std::find_if(frameData[currentFrame].materialsIndicesAllocation.begin(), frameData[currentFrame].materialsIndicesAllocation.end(),
            [&](const Resource::id_t &id) {
                return id == Resource::INVALID_ID;
            });
        if (it == frameData[currentFrame].materialsIndicesAllocation.end())
            die("Maximum number of materials reached");
        const auto index = it - frameData[currentFrame].materialsIndicesAllocation.begin();
        frameData[currentFrame].materialsIndices[material->getId()] = index;
        frameData[currentFrame].materialsIndicesAllocation[index] = material->getId();
        // Force material data to be written to GPU memory
        material->_setDirty();
        frameData[currentFrame].materials.push_back(material);
    }

    void SceneRenderer::removeMaterial(const shared_ptr<Material> &material, const uint32_t currentFrame) {
        if (frameData[currentFrame].materialsIndices.contains(material->getId())) {
            // Check if we need to remove the material from the scene
            if (material->_decrementReferenceCounter()) {
                // Try to remove the associated textures or shaders
                if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                    if (standardMaterial->getAlbedoTexture() != nullptr)
                        removeImage(standardMaterial->getAlbedoTexture()->getImage(), currentFrame);
                    if (standardMaterial->getSpecularTexture() != nullptr)
                        removeImage(standardMaterial->getSpecularTexture()->getImage(), currentFrame);
                    if (standardMaterial->getNormalTexture() != nullptr)
                        removeImage(standardMaterial->getNormalTexture()->getImage(), currentFrame);
                } else if (const auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material.get())) {
                    const auto &shader = frameData[currentFrame].materialShaders[shaderMaterial->getFragFileName()];
                    if (shader->_decrementReferenceCounter()) {
                        frameData[currentFrame].materialShaders.erase(shaderMaterial->getFragFileName());
                    }
                }
                // Free the material index and remove the material from the scene
                const auto index = frameData[currentFrame].materialsIndices[material->getId()];
                frameData[currentFrame].materialsIndicesAllocation[index] = Resource::INVALID_ID;
                frameData[currentFrame].materialsIndices.erase(material->getId());
                frameData[currentFrame].materials.remove(material);
            }
        }
    }

    void SceneRenderer::postUpdateScene(const uint32_t currentFrame) {
        createOrUpdateResources(true, &pushConstantRange);
        for (const auto &material : OutlineMaterials::_all()) {
            loadShadersMaterials(material.get(), currentFrame);
        }
        if (currentFrame == 0) {
            for (const auto &renderer : shadowMapRenderers) {
                renderer->loadScene(ModelsRenderer::frameData[currentFrame].models);
            }
        }
    }

    void SceneRenderer::addingModel(MeshInstance *meshInstance, const uint32_t modelIndex, const uint32_t currentFrame) {
        frameData[currentFrame].opaquesModels.push_back(meshInstance);
        frameData[currentFrame].modelsIndices[meshInstance->getId()] = modelIndex;
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            material->_incrementReferenceCounter();
            if (frameData[currentFrame].materialsIndices.contains(material->getId())) continue;
            addMaterial(material, currentFrame);
            // Load textures for standards materials
            if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                if (standardMaterial->getAlbedoTexture() != nullptr)
                    addImage(standardMaterial->getAlbedoTexture()->getImage(), currentFrame);
                if (standardMaterial->getSpecularTexture() != nullptr)
                    addImage(standardMaterial->getSpecularTexture()->getImage(), currentFrame);
                if (standardMaterial->getNormalTexture() != nullptr)
                    addImage(standardMaterial->getNormalTexture()->getImage(), currentFrame);
            }
        }
    }

    void SceneRenderer::addedModel(MeshInstance *meshInstance, const uint32_t currentFrame) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            if (const auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material.get())) {
                loadShadersMaterials(shaderMaterial, currentFrame);
            }
        }
        frameData[currentFrame].opaquesModels.sort([](const MeshInstance *a, const MeshInstance*b) { return *a < *b; });
    }

    void SceneRenderer::removingModel(MeshInstance *meshInstance, const uint32_t currentFrame) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
           removeMaterial(material, currentFrame);
        }

        const auto it = find(frameData[currentFrame].opaquesModels.begin(), frameData[currentFrame].opaquesModels.end(), meshInstance);
        if (it != frameData[currentFrame].opaquesModels.end()) {
            frameData[currentFrame].opaquesModels.erase(it);
            frameData[currentFrame].modelsIndices.erase(meshInstance->getId());
        }
    }

    void SceneRenderer::loadShadersMaterials(const ShaderMaterial *material, const uint32_t currentFrame) {
        if (!frameData[currentFrame].materialShaders.contains(material->getFragFileName())) {
            if (!material->getVertFileName().empty()) {
                frameData[currentFrame].materialShaders[material->getVertFileName()] = createShader(
                        material->getVertFileName(), VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
                frameData[currentFrame].materialShaders[material->getVertFileName()]->_incrementReferenceCounter();
            }
            if (!material->getFragFileName().empty()) {
                frameData[currentFrame].materialShaders[material->getFragFileName()] =
                        createShader(material->getFragFileName(), VK_SHADER_STAGE_FRAGMENT_BIT, 0);
                frameData[currentFrame].materialShaders[material->getFragFileName()]->_incrementReferenceCounter();
            }
        }
    }

    void SceneRenderer::activateCamera(Camera *camera, const uint32_t currentFrame) {
        ModelsRenderer::activateCamera(camera, currentFrame);
        if (currentFrame == 0) {
            for (const auto &renderer : shadowMapRenderers) {
                renderer->activateCamera(camera);
            }
        }
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        if (ModelsRenderer::frameData[currentFrame].currentCamera == nullptr)
            return;
        if (frameData[currentFrame].skyboxRenderer != nullptr)
            frameData[currentFrame].skyboxRenderer->update(ModelsRenderer::frameData[currentFrame].currentCamera, frameData[currentFrame].currentEnvironment, currentFrame);
        if (ModelsRenderer::frameData[currentFrame].models.empty())
            return;

        GobalUniformBuffer globalUbo{
                .projection           = ModelsRenderer::frameData[currentFrame].currentCamera->getProjection(),
                .view                 = ModelsRenderer::frameData[currentFrame].currentCamera->getView(),
                .cameraPosition       = ModelsRenderer::frameData[currentFrame].currentCamera->getPositionGlobal(),
                .haveDirectionalLight = frameData[currentFrame].directionalLight != nullptr,
                .pointLightsCount     = static_cast<uint32_t>(frameData[currentFrame].omniLights.size()),
                .shadowMapsCount      = static_cast<uint32_t>(shadowMapRenderers.size()),
        };
        if (globalUbo.haveDirectionalLight) {
            globalUbo.directionalLight = {
                    .direction = frameData[currentFrame].directionalLight->getFrontVector(),
                    .color    = frameData[currentFrame].directionalLight->getColorAndIntensity(),
                    .specular = frameData[currentFrame].directionalLight->getSpecularIntensity(),
            };
        }
        if (frameData[currentFrame].currentEnvironment != nullptr) {
            globalUbo.ambient = frameData[currentFrame].currentEnvironment->getAmbientColorAndIntensity();
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
            writeUniformBuffer(frameData[currentFrame].shadowMapsUniformBuffer, shadowMapArray.get());
        }
        writeUniformBuffer(globalUniformBuffers[currentFrame], &globalUbo); // after the shadows maps for .cascadedShadowMapIndex

        if (globalUbo.pointLightsCount > 0) {
            auto pointLightsArray = make_unique<PointLightUniformBuffer[]>(globalUbo.pointLightsCount);
            auto lightIndex = 0;
            for (const auto* omniLight : frameData[currentFrame].omniLights) {
                pointLightsArray[lightIndex].position  = omniLight->getPositionGlobal();
                pointLightsArray[lightIndex].color     = omniLight->getColorAndIntensity();
                pointLightsArray[lightIndex].specular  = omniLight->getSpecularIntensity();
                pointLightsArray[lightIndex].constant  = omniLight->getAttenuation();
                pointLightsArray[lightIndex].linear    = omniLight->getLinear();
                pointLightsArray[lightIndex].quadratic = omniLight->getQuadratic();
                if (const auto *spot = dynamic_cast<const SpotLight *>(omniLight)) {
                    pointLightsArray[lightIndex].isSpot    = true;
                    pointLightsArray[lightIndex].direction = normalize(mat3{spot->getTransformGlobal()} * AXIS_FRONT);
                    pointLightsArray[lightIndex].cutOff    = spot->getCutOff();
                    pointLightsArray[lightIndex].outerCutOff = spot->getOuterCutOff();
                }
                lightIndex += 1;
            }
            writeUniformBuffer(frameData[currentFrame].pointLightUniformBuffer,pointLightsArray.get());
        }

        uint32_t modelIndex = 0;
        auto modelUBOArray = make_unique<ModelUniformBuffer[]>(ModelsRenderer::frameData[currentFrame].models.size());
        for (const auto &meshInstance : ModelsRenderer::frameData[currentFrame].models) {
            modelUBOArray[modelIndex].matrix = meshInstance->getTransformGlobal();
            modelIndex += 1;
        }
        writeUniformBuffer(ModelsRenderer::frameData[currentFrame].modelUniformBuffer, modelUBOArray.get());

        auto materialIndex = 0;
        auto materialUBOArray = make_unique<MaterialUniformBuffer[]>(ModelsRenderer::frameData[currentFrame].models.size());
        for (const auto& material : frameData[currentFrame].materials) {
            // if (material->_isDirty()) {
                materialUBOArray[materialIndex].transparency = material->getTransparency();
                materialUBOArray[materialIndex].alphaScissor = material->getAlphaScissor();
                if (auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                    materialUBOArray[materialIndex].albedoColor = standardMaterial->getAlbedoColor().color;
                    if (standardMaterial->getAlbedoTexture() != nullptr) {
                        materialUBOArray[materialIndex].diffuseIndex = frameData[currentFrame].imagesIndices[standardMaterial->getAlbedoTexture()->getImage()->getId()];
                        const auto &transform    = standardMaterial->getTextureTransform();
                        if (transform != nullptr) {
                            materialUBOArray[materialIndex].hasTransform  = true;
                            materialUBOArray[materialIndex].textureOffset = transform->offset;
                            materialUBOArray[materialIndex].textureScale  = transform->scale;
                        }
                    }
                    if (standardMaterial->getSpecularTexture() != nullptr) {
                        materialUBOArray[materialIndex].specularIndex =
                                frameData[currentFrame].imagesIndices[standardMaterial->getSpecularTexture()->getImage()->getId()];
                    }
                    if (standardMaterial->getNormalTexture() != nullptr) {
                        materialUBOArray[materialIndex].normalIndex = frameData[currentFrame].imagesIndices[standardMaterial->getNormalTexture()->getImage()->getId()];
                    }
                } else if (auto *shaderMaterial = dynamic_cast<ShaderMaterial *>(material.get())) {
                    for (int i = 0; i < ShaderMaterial::MAX_PARAMETERS; i++) {
                        materialUBOArray[materialIndex].parameters[i] = shaderMaterial->getParameter(i);
                    }
                }
                materialIndex+= 1;
                // material->_clearDirty();
                // frameData[currentFrame].materialsBuffer->writeToBuffer(
                //     &materialUBOArray[materialIndex],
                //     materialBufferSize,
                //     materialBufferSize * material->_getBufferIndex());
            // }
        }
        writeUniformBuffer(frameData[currentFrame].materialsBuffer, materialUBOArray.get());
    }

    void SceneRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        if (ModelsRenderer::frameData[currentFrame].currentCamera == nullptr)
            return;
        setInitialState(commandBuffer, currentFrame);
        if (!ModelsRenderer::frameData[currentFrame].models.empty()) {
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            bindDescriptorSets(commandBuffer, currentFrame);
            drawModels(commandBuffer, currentFrame, frameData[currentFrame].opaquesModels);
        }
        if (frameData[currentFrame].skyboxRenderer != nullptr)
            frameData[currentFrame].skyboxRenderer->recordCommands(commandBuffer, currentFrame);
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
        if (blankImage == nullptr) { blankImage = Image::createBlankImage(); }

        globalUniformBufferSize = sizeof(GobalUniformBuffer);
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            globalUniformBuffers[i] = createUniformBuffer(globalUniformBufferSize);
        }
    }

    void SceneRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (!ModelsRenderer::frameData[i].models.empty() && (frameData[i].modelUniformBufferCount != ModelsRenderer::frameData[i].models.size())) {
                frameData[i].modelUniformBufferCount = ModelsRenderer::frameData[i].models.size();
                ModelsRenderer::frameData[i].modelUniformBuffer = createUniformBuffer(modelUniformBufferSize * frameData[i].modelUniformBufferCount);
            }
            if (frameData[i].modelUniformBufferCount == 0) {
                frameData[i].modelUniformBufferCount = 1;
                ModelsRenderer::frameData[i].modelUniformBuffer = createUniformBuffer(modelUniformBufferSize);
            }
            if (frameData[i].materialsBuffer == nullptr) {
                frameData[i].materialsBuffer = createUniformBuffer(materialBufferSize * MAX_MATERIALS);
            }

            if ((!shadowMapRenderers.empty()) && (frameData[i].shadowMapUniformBufferCount != shadowMapRenderers.size())) {
                frameData[i].shadowMapUniformBufferCount = shadowMapRenderers.size();
                frameData[i].shadowMapsUniformBuffer = createUniformBuffer(shadowMapUniformBufferSize * frameData[i].shadowMapUniformBufferCount);
            }
            if (frameData[i].shadowMapUniformBufferCount == 0) {
                frameData[i].shadowMapUniformBufferCount = 1;
                frameData[i].shadowMapsUniformBuffer = createUniformBuffer(shadowMapUniformBufferSize);
            }
            if ((!frameData[i].omniLights.empty()) && (frameData[i].pointLightUniformBufferCount != frameData[i].omniLights.size())) {
                frameData[i].pointLightUniformBufferCount = frameData[i].omniLights.size();
                frameData[i].pointLightUniformBuffer = createUniformBuffer(pointLightUniformBufferSize * frameData[i].pointLightUniformBufferCount);
            }
            if (frameData[i].pointLightUniformBufferCount == 0) {
                frameData[i].pointLightUniformBufferCount = 1;
                frameData[i].pointLightUniformBuffer = createUniformBuffer(pointLightUniformBufferSize);
            }

            uint32_t imageIndex = 0;
            for (const auto &image : frameData[i].images) {
                frameData[i].imagesInfo[imageIndex] = image->_getImageInfo();
                imageIndex += 1;
            }
            // initialize the rest of the image info array with the blank image
            for (uint32_t j = imageIndex; j < frameData[i].imagesInfo.size(); j++) {
                frameData[i].imagesInfo[j] = blankImage->_getImageInfo();
            }

            imageIndex = 0;
            for (const auto &shadowMapRenderer : shadowMapRenderers) {
                const auto &shadowMap      = shadowMapRenderer->getShadowMap();
                frameData[i].shadowMapsInfo[imageIndex] = VkDescriptorImageInfo{
                        .sampler     = shadowMap->getSampler(),
                        .imageView   = shadowMap->FrameBuffer::getImageView(),
                        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                };
                imageIndex += 1;
            }
            // initialize the rest of the shadow map info array with the blank image
            for (uint32_t j = imageIndex; j < frameData[i].shadowMapsInfo.size(); j++) {
                frameData[i].shadowMapsInfo[j] = blankImage->_getImageInfo();
            }

            auto globalBufferInfo     = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto modelBufferInfo      = ModelsRenderer::frameData[i].modelUniformBuffer->descriptorInfo(modelUniformBufferSize *
                                                                                frameData[i].modelUniformBufferCount);
            auto materialBufferInfo   = frameData[i].materialsBuffer->descriptorInfo(materialBufferSize * MAX_MATERIALS);
            auto shadowMapBufferInfo  = frameData[i].shadowMapsUniformBuffer->descriptorInfo(shadowMapUniformBufferSize *
                                                                                   frameData[i].shadowMapUniformBufferCount);
            auto pointLightBufferInfo = frameData[i].pointLightUniformBuffer->descriptorInfo(pointLightUniformBufferSize *
                                                                                   frameData[i].pointLightUniformBufferCount);
            auto writer               = DescriptorWriter(*setLayout, *descriptorPool)
                                  .writeBuffer(0, &globalBufferInfo)
                                  .writeBuffer(1, &modelBufferInfo)
                                  .writeBuffer(2, &materialBufferInfo)
                                  .writeImage(3, frameData[i].imagesInfo.data())
                                  .writeBuffer(4, &shadowMapBufferInfo)
                                  .writeImage(5, frameData[i].shadowMapsInfo.data())
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
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (frameData[i].skyboxRenderer != nullptr) {
                frameData[i].skyboxRenderer->loadShaders();
            }
        }
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::createImagesResources() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            colorFrameBufferHdr[i] = make_shared<ColorFrameBufferHDR>(device);
            if (ModelsRenderer::frameData[i].depthFrameBuffer == nullptr) {
                ModelsRenderer::frameData[i].depthFrameBuffer         = make_shared<DepthFrameBuffer>(device, true);
                frameData[i].resolvedDepthFrameBuffer = make_shared<DepthFrameBuffer>(device, false);
            } else {
                ModelsRenderer::frameData[i].depthFrameBuffer->createImagesResources();
            }
        }
    }

    void SceneRenderer::cleanupImagesResources() {
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (ModelsRenderer::frameData[i].depthFrameBuffer != nullptr) {
                frameData[i].resolvedDepthFrameBuffer->cleanupImagesResources();
            }
            colorFrameBufferHdr[i]->cleanupImagesResources();
            frameData[i].colorFrameBufferMultisampled->cleanupImagesResources();
        }
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        for (auto i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            colorFrameBufferHdr[i]->createImagesResources();
             frameData[i].colorFrameBufferMultisampled->createImagesResources();
            if (ModelsRenderer::frameData[i].depthFrameBuffer != nullptr) {
                 frameData[i].resolvedDepthFrameBuffer->createImagesResources();
            }
        }
    }

    void SceneRenderer::beginRendering(const VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        // https://lesleylai.info/en/vk-khr-dynamic-rendering/
        Device::transitionImageLayout(commandBuffer,
                                      frameData[currentFrame].colorFrameBufferMultisampled->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      0,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr[currentFrame]->getImage(),
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
                .imageView          = frameData[currentFrame].colorFrameBufferMultisampled->getImageView(),
                .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT,
                .resolveImageView   = colorFrameBufferHdr[currentFrame]->getImageView(),
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue         = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView          = ModelsRenderer::frameData[currentFrame].depthFrameBuffer->getImageView(),
                .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT,
                .resolveImageView   = frameData[currentFrame].resolvedDepthFrameBuffer->getImageView(),
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

    void SceneRenderer::endRendering(const VkCommandBuffer commandBuffer, uint32_t currentFrame, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer,
                                      colorFrameBufferHdr[currentFrame]->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                                             : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      0,
                                      isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                      isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer,
                                       frameData[currentFrame].resolvedDepthFrameBuffer->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                                      0,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void SceneRenderer::addImage(const shared_ptr<Image> &image, const uint32_t currentFrame) {
        image->_incrementReferenceCounter();
        if (find(frameData[currentFrame].images.begin(), frameData[currentFrame].images.end(), image.get()) != frameData[currentFrame].images.end())
            return;
        if (frameData[currentFrame].images.size() == MAX_IMAGES)
            die("Maximum images count reached for the scene renderer");
        frameData[currentFrame].imagesIndices[image->getId()] = static_cast<int32_t>(frameData[currentFrame].images.size());
        frameData[currentFrame].images.push_back(image.get());
    }

    void SceneRenderer::removeImage(const shared_ptr<Image> &image, const uint32_t currentFrame) {
        if (find(frameData[currentFrame].images.begin(), frameData[currentFrame].images.end(), image.get()) != frameData[currentFrame].images.end()) {
            if (image->_decrementReferenceCounter()) {
                frameData[currentFrame].images.remove(image.get());
                // Rebuild the image index
                frameData[currentFrame].imagesIndices.clear();
                uint32_t imageIndex = 0;
                for (const auto &img : frameData[currentFrame].images) {
                    frameData[currentFrame].imagesIndices[img->getId()] = static_cast<int32_t>(imageIndex);
                    imageIndex += 1;
                }
            }
        }
    }

    void SceneRenderer::drawModels(const VkCommandBuffer commandBuffer,
                                   const uint32_t currentFrame,
                                   const list<MeshInstance *> &modelsToDraw) {
        auto shadersChanged = false;
        const auto cameraFrustum = Frustum{
            ModelsRenderer::frameData[currentFrame].currentCamera,
            ModelsRenderer::frameData[currentFrame].currentCamera->getFov(),
            ModelsRenderer::frameData[currentFrame].currentCamera->getNearClipDistance(),
            ModelsRenderer::frameData[currentFrame].currentCamera->getFarClipDistance()
        };

        // Used to reduce vkCmdSetCullMode calls
        auto lastCullMode = CULLMODE_BACK;
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);

        // Used to reduce vkCmdBindVertexBuffers & vkCmdBindIndexBuffer calls
        auto lastMeshId = Resource::id_t{numeric_limits<uint32_t>::max()};

        for (const auto &meshInstance : modelsToDraw) {
            if (meshInstance->isValid() && cameraFrustum.isOnFrustum(meshInstance)) {
                const auto &modelIndex = frameData[currentFrame].modelsIndices[meshInstance->getId()];
                const auto &model      = meshInstance->getMesh();
                for (const auto &surface : model->getSurfaces()) {
                    if (meshInstance->isOutlined()) {
                        const auto &material = meshInstance->getOutlineMaterial();
                        auto pushConstants = PushConstants {
                            .modelIndex = static_cast<int>(modelIndex),
                            .materialIndex = frameData[currentFrame].materialsIndices[material->getId()]
                        };
                        vkCmdBindShadersEXT(commandBuffer,
                                            1,
                                            frameData[currentFrame].materialShaders[material->getVertFileName()]->getStage(),
                                            frameData[currentFrame].materialShaders["outline.vert"]->getShader());
                        vkCmdBindShadersEXT(commandBuffer,
                                            1,
                                            frameData[currentFrame].materialShaders[material->getFragFileName()]->getStage(),
                                            frameData[currentFrame].materialShaders["outline.frag"]->getShader());
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT);
                        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL_GRAPHICS,
                                            0, sizeof(PushConstants), &pushConstants);
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
                            Shader *material = frameData[currentFrame].materialShaders[shaderMaterial->getFragFileName()].get();
                            vkCmdBindShadersEXT(commandBuffer, 1, material->getStage(), material->getShader());
                            shadersChanged = true;
                        }
                        if (!shaderMaterial->getVertFileName().empty()) {
                            Shader *material = frameData[currentFrame].materialShaders[shaderMaterial->getVertFileName()].get();
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
                        .materialIndex = frameData[currentFrame].materialsIndices[surface->material->getId()]
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
        if (enableShadowMapRenders) {
            if (const auto renderer = findShadowMapRenderer(light); renderer == nullptr) {
                if (light->getCastShadows() && (shadowMapRenderers.size() < MAX_SHADOW_MAPS)) {
                    const auto shadowMapRenderer = make_shared<ShadowMapRenderer>(device, shaderDirectory, light);
                    shadowMapRenderer->activateCamera(ModelsRenderer::frameData[0].currentCamera);
                    shadowMapRenderers.push_back(shadowMapRenderer);
                    device.registerRenderer(shadowMapRenderer);
                }
            }
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

    vector<ColorFrameBufferHDR*> SceneRenderer::getSampledAttachments() const {
        auto result = vector<ColorFrameBufferHDR*>{MAX_FRAMES_IN_FLIGHT};
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            result[i] = colorFrameBufferHdr[i].get();
        }
        return result;
    }

} // namespace z0
