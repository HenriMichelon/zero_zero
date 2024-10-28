module;
#include <cassert>
#include <glm/detail/type_mat3x4.hpp>
#include <json.hpp>
#include <volk.h>

#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Node;
import :MeshInstance;
import :Environment;
import :Material;
import :Resource;
import :Image;
import :Cubemap;
import :Light;
import :DirectionalLight;
import :OmniLight;
import :SpotLight;
import :Skybox;
import :FrustumCulling;

import :ModelsRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;
import :ShadowMapRenderer;
import :SkyboxRenderer;
import :Buffer;
import :Shader;
import :FrameBuffer;
import :ShadowMapFrameBuffer;
import :ColorFrameBuffer;
import :DepthFrameBuffer;
import :SceneRenderer;
import :SampledFrameBuffer;
import :VulkanCubemap;
import :VulkanImage;
import :VulkanMesh;

namespace z0 {

    SceneRenderer::SceneRenderer(Device &device, const string &shaderDirectory, const vec3 clearColor) :
        ModelsRenderer{device, shaderDirectory, clearColor} {
        frameData.resize(device.getFramesInFlight());
        colorFrameBufferHdr.resize(device.getFramesInFlight());
        resolvedDepthFrameBuffer.resize(device.getFramesInFlight());
        createImagesResources();
        OutlineMaterials::_initialize();
        for(auto& frame: frameData) {
            frame.colorFrameBufferMultisampled = make_unique<ColorFrameBuffer>(device, true);
            frame.materialsIndicesAllocation = vector(MAX_MATERIALS, Resource::INVALID_ID);
        }
        createOrUpdateResources(true, &pushConstantRange);
    }

    void SceneRenderer::cleanup() {
        blankImage.reset();
        blankCubemap.reset();
        for(auto& frame: frameData) {
            frame.globalBuffer.reset();
            frame.materialShaders.clear();
            frame.materialsBuffer.reset();
            frame.lightBuffer.reset();
            if (frame.skyboxRenderer != nullptr)
                frame.skyboxRenderer->cleanup();
            frame.opaquesModels.clear();
            frame.lights.clear();
        }
        for (const auto &pair : shadowMapRenderers) {
            device.unRegisterRenderer(pair.second);
            pair.second->cleanup();
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
            if (const auto& environment = dynamic_pointer_cast<Environment>(node)) {
                frameData[currentFrame].currentEnvironment = environment;
                return;
            }
        }
        if (const auto& light = dynamic_pointer_cast<Light>(node)) {
            frameData[currentFrame].lights.push_back(light);
            descriptorSetNeedUpdate = true;
            enableLightShadowCasting(light);
        }
        ModelsRenderer::addNode(node, currentFrame);
    }

    void SceneRenderer::removeNode(const shared_ptr<Node> &node, const uint32_t currentFrame) {
        if (dynamic_cast<Skybox *>(node.get())) {
            frameData[currentFrame].skyboxRenderer.reset();
        } else if (const auto& environment = dynamic_pointer_cast<Environment>(node)) {
            if (frameData[currentFrame].currentEnvironment == environment) {
                frameData[currentFrame].currentEnvironment = nullptr;
            }
        } else if (const auto& light = dynamic_pointer_cast<Light>(node)) {
            disableLightShadowCasting(light);
            std::erase(frameData[currentFrame].lights, light);
        } else {
            ModelsRenderer::removeNode(node, currentFrame);
        }
    }

    void SceneRenderer::preUpdateScene(const uint32_t currentFrame) {
        for (const auto &material : OutlineMaterials::_all()) {
            if (!frameData[currentFrame].materialsIndices.contains(material->getId())) {
                addMaterial(material, currentFrame);
                descriptorSetNeedUpdate = true;
            }
        }
    }

    void SceneRenderer::addMaterial(const shared_ptr<Material> &material, const uint32_t currentFrame) {
        // Allocate the first free buffer index
        const auto it = std::find_if(
            frameData[currentFrame].materialsIndicesAllocation.begin(),
            frameData[currentFrame].materialsIndicesAllocation.end(),
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
        frameData[currentFrame].materialsRefCounter[material->getId()]++;
    }

    void SceneRenderer::removeMaterial(const shared_ptr<Material> &material, const uint32_t currentFrame) {
        if (frameData[currentFrame].materialsIndices.contains(material->getId())) {
            // Check if we need to remove the material from the scene
            if (--frameData[currentFrame].materialsRefCounter[material->getId()] == 0) {
                // Try to remove the associated textures or shaders
                if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                    if (standardMaterial->getAlbedoTexture().texture != nullptr)
                        removeImage(standardMaterial->getAlbedoTexture().texture->getImage(), currentFrame);
                    if (standardMaterial->getNormalTexture().texture != nullptr)
                        removeImage(standardMaterial->getNormalTexture().texture->getImage(), currentFrame);
                    if (standardMaterial->getMetallicTexture().texture != nullptr)
                        removeImage(standardMaterial->getMetallicTexture().texture->getImage(), currentFrame);
                    if (standardMaterial->getRoughnessTexture().texture != nullptr)
                        removeImage(standardMaterial->getRoughnessTexture().texture->getImage(), currentFrame);
                    if (standardMaterial->getAmbientOcclusionTexture().texture != nullptr)
                        removeImage(standardMaterial->getAmbientOcclusionTexture().texture->getImage(), currentFrame);
                    if (standardMaterial->getEmissiveTexture().texture != nullptr)
                        removeImage(standardMaterial->getEmissiveTexture().texture->getImage(), currentFrame);
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
            loadShadersMaterials(material, currentFrame);
        }
        for (const auto &pair : shadowMapRenderers) {
            if (!pair.second->isInitialized()) {
                pair.second->loadScene(ModelsRenderer::frameData[currentFrame].models);
            }
        }
    }

    void SceneRenderer::addingModel(const shared_ptr<MeshInstance>& meshInstance, const uint32_t modelIndex, const uint32_t currentFrame) {
        frameData[currentFrame].opaquesModels.push_back(meshInstance);
        frameData[currentFrame].modelsIndices[meshInstance->getId()] = modelIndex;
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            if (frameData[currentFrame].materialsIndices.contains(material->getId())) {
                frameData[currentFrame].materialsRefCounter[material->getId()]++;
                continue;
            }
            addMaterial(material, currentFrame);
            // Load textures for standards materials
            if (const auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                if (standardMaterial->getAlbedoTexture().texture != nullptr)
                    addImage(standardMaterial->getAlbedoTexture().texture->getImage(), currentFrame);
                if (standardMaterial->getNormalTexture().texture != nullptr)
                    addImage(standardMaterial->getNormalTexture().texture->getImage(), currentFrame);
                if (standardMaterial->getMetallicTexture().texture != nullptr)
                    addImage(standardMaterial->getMetallicTexture().texture->getImage(), currentFrame);
                if (standardMaterial->getRoughnessTexture().texture != nullptr)
                    addImage(standardMaterial->getRoughnessTexture().texture->getImage(), currentFrame);
                if (standardMaterial->getAmbientOcclusionTexture().texture != nullptr)
                    addImage(standardMaterial->getAmbientOcclusionTexture().texture->getImage(), currentFrame);
                if (standardMaterial->getEmissiveTexture().texture != nullptr)
                    addImage(standardMaterial->getEmissiveTexture().texture->getImage(), currentFrame);
            }
        }
    }

    void SceneRenderer::addedModel(const shared_ptr<MeshInstance>&meshInstance, const uint32_t currentFrame) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
            if (const auto& shaderMaterial = dynamic_pointer_cast<ShaderMaterial>(material)) {
                loadShadersMaterials(shaderMaterial, currentFrame);
            }
        }
        frameData[currentFrame].opaquesModels.sort([](const shared_ptr<MeshInstance>&a, const shared_ptr<MeshInstance>&b) { return *a < *b; });
    }

    void SceneRenderer::removingModel(const shared_ptr<MeshInstance>&meshInstance, const uint32_t currentFrame) {
        for (const auto &material : meshInstance->getMesh()->_getMaterials()) {
           removeMaterial(material, currentFrame);
        }

        const auto it = find(frameData[currentFrame].opaquesModels.begin(), frameData[currentFrame].opaquesModels.end(), meshInstance);
        if (it != frameData[currentFrame].opaquesModels.end()) {
            frameData[currentFrame].opaquesModels.erase(it);
            frameData[currentFrame].modelsIndices.erase(meshInstance->getId());
        }
    }

    void SceneRenderer::loadShadersMaterials(const shared_ptr<ShaderMaterial>&material, const uint32_t currentFrame) {
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

    void SceneRenderer::activateCamera(const shared_ptr<Camera>& camera, const uint32_t currentFrame) {
        ModelsRenderer::activateCamera(camera, currentFrame);
        for (const auto &pair : shadowMapRenderers) {
            pair.second->activateCamera(camera, currentFrame);
        }
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        const auto& frame = frameData[currentFrame];
        if (ModelsRenderer::frameData[currentFrame].currentCamera == nullptr) { return; }
        if (frame.skyboxRenderer != nullptr) {
            frame.skyboxRenderer->update(ModelsRenderer::frameData[currentFrame].currentCamera, frameData[currentFrame].currentEnvironment, currentFrame);
        }
        if (ModelsRenderer::frameData[currentFrame].models.empty()) { return; }

        GlobalBuffer globalUbo{
            .projection      = ModelsRenderer::frameData[currentFrame].currentCamera->getProjection(),
            .view            = ModelsRenderer::frameData[currentFrame].currentCamera->getView(),
            .cameraPosition  = ModelsRenderer::frameData[currentFrame].currentCamera->getPositionGlobal(),
            .lightsCount     = static_cast<uint32_t>(frame.lights.size()),
            .ambient         = frame.currentEnvironment != nullptr ? frame.currentEnvironment->getAmbientColorAndIntensity() : vec4{1.0f},
            .ambientIBL      = static_cast<uint32_t>((frame.skyboxRenderer != nullptr) && (frame.skyboxRenderer->getCubemap()->getCubemapType() == Cubemap::TYPE_ENVIRONMENT) ? 1: 0),
        };
        writeUniformBuffer(frame.globalBuffer, &globalUbo);

        if (globalUbo.lightsCount > 0) {
            auto lightsArray = make_unique<LightBuffer[]>(globalUbo.lightsCount);
            auto lightIndex = 0;
            for (const auto& light : frame.lights) {
                lightsArray[lightIndex].type      = light->getLightType();
                lightsArray[lightIndex].position  = light->getPositionGlobal();
                lightsArray[lightIndex].color     = light->getColorAndIntensity();
                lightsArray[lightIndex].specular  = light->getSpecularIntensity();
                switch (light->getLightType()) {
                    case Light::LIGHT_DIRECTIONAL: {
                        const auto& directionalLight = reinterpret_pointer_cast<DirectionalLight>(light);
                        lightsArray[lightIndex].direction = normalize(mat3{directionalLight->getTransformGlobal()} * AXIS_FRONT);
                        break;
                    }
                    case Light::LIGHT_SPOT: {
                        const auto& spotLight = reinterpret_pointer_cast<SpotLight>(light);
                        lightsArray[lightIndex].direction = normalize(mat3{spotLight->getTransformGlobal()} * AXIS_FRONT);
                        lightsArray[lightIndex].cutOff    = spotLight->getCutOff();
                        lightsArray[lightIndex].outerCutOff = spotLight->getOuterCutOff();
                        // a spot is also an omni
                    }
                    case Light::LIGHT_OMNI: {
                        const auto& omniLight = reinterpret_pointer_cast<OmniLight>(light);
                        lightsArray[lightIndex].range  = omniLight->getRange();
                        break;
                    }
                    default:
                        assert(false);
                }
                if (shadowMapRenderers.contains(light)) {
                    const auto& shadowMapRenderer = shadowMapRenderers.at(light);
                    lightsArray[lightIndex].mapIndex = shadowMapRenderer->getShadowMap(currentFrame)->_getBufferIndex();
                    switch (light->getLightType()) {
                        case Light::LIGHT_DIRECTIONAL: {
                            lightsArray[lightIndex].cascadesCount = shadowMapRenderer->getCascadesCount(currentFrame);
                            for (int cascadeIndex = 0; cascadeIndex < lightsArray[lightIndex].cascadesCount ; cascadeIndex++) {
                                    lightsArray[lightIndex].lightSpace[cascadeIndex] = shadowMapRenderer->getLightSpace(cascadeIndex, currentFrame);
                                    lightsArray[lightIndex].cascadeSplitDepth[cascadeIndex] = shadowMapRenderer->getCascadeSplitDepth(cascadeIndex, currentFrame);
                            }
                            break;
                        }
                        case Light::LIGHT_SPOT: {
                            // Just copy the light space matrix for non cascaded shadow maps
                            lightsArray[lightIndex].lightSpace[0] = shadowMapRenderer->getLightSpace(0, currentFrame);
                            break;
                        }
                        case Light::LIGHT_OMNI: {
                            lightsArray[lightIndex].farPlane = shadowMapRenderer->getFarPlane();
                            for (int faceIndex = 0; faceIndex < 6; faceIndex++) {
                                lightsArray[lightIndex].lightSpace[faceIndex] =shadowMapRenderer->getLightSpace(faceIndex, currentFrame);
                            }
                            break;
                        }
                    }
                }
                lightIndex += 1;
            }
            writeUniformBuffer(frame.lightBuffer, lightsArray.get());
        }

        uint32_t modelIndex = 0;
        auto modelUBOArray = make_unique<ModelBuffer[]>(ModelsRenderer::frameData[currentFrame].models.size());
        for (const auto &meshInstance : ModelsRenderer::frameData[currentFrame].models) {
            modelUBOArray[modelIndex].matrix = meshInstance->getTransformGlobal();
            modelIndex += 1;
        }
        writeUniformBuffer(ModelsRenderer::frameData[currentFrame].modelUniformBuffer, modelUBOArray.get());

        // Update in GPU memory only the materials modified since the last frame
        for (const auto& material : frame.materials) {
            if (!material->_isDirty()) { continue; }
            auto materialUBO = MaterialBuffer{
                .transparency = material->getTransparency(),
                .alphaScissor = material->getAlphaScissor()
            };
            const auto materialIndex = frame.materialsIndices.at(material->getId());
            if (const auto *standardMaterial = dynamic_cast<const StandardMaterial *>(material.get())) {
                materialUBO.albedoColor = standardMaterial->getAlbedoColor().color;
                materialUBO.metallicFactor = standardMaterial->getMetallicFactor();
                materialUBO.roughnessFactor = standardMaterial->getRoughnessFactor();
                materialUBO.emissiveFactor = standardMaterial->getEmissiveFactor();
                materialUBO.emissiveStrength = standardMaterial->getEmissiveStrength();
                materialUBO.normalScale = standardMaterial->getNormaleScale();
                auto convert = [&](TextureInfo& dest, const StandardMaterial::TextureInfo& texInfo) {
                    dest.offset = texInfo.offset;
                    dest.scale = texInfo.scale;
                    if (texInfo.texture != nullptr) {
                        dest.index = frame.imagesIndices.at(texInfo.texture->getImage()->getId());
                    }
                };
                auto textureUBO = TextureBuffer{};
                convert(textureUBO.albedoTexture, standardMaterial->getAlbedoTexture());
                // convert(textureUBO.specularTexture, standardMaterial->getSpecularTexture());
                convert(textureUBO.normalTexture, standardMaterial->getNormalTexture());
                convert(textureUBO.metallicTexture, standardMaterial->getMetallicTexture());
                convert(textureUBO.roughnessTexture, standardMaterial->getRoughnessTexture());
                convert(textureUBO.ambientOcclusionTexture, standardMaterial->getAmbientOcclusionTexture());
                convert(textureUBO.emissiveTexture, standardMaterial->getEmissiveTexture());
                frame.texturesBuffer->writeToBuffer(
                    &textureUBO,
                    TEXTURE_BUFFER_SIZE,
                    TEXTURE_BUFFER_SIZE * materialIndex);
            } else if (const auto *shaderMaterial = dynamic_cast<const ShaderMaterial *>(material.get())) {
                for (auto i = 0; i < ShaderMaterial::MAX_PARAMETERS; i++) {
                   materialUBO.parameters[i] = shaderMaterial->getParameter(i);
                }
            }
            frame.materialsBuffer->writeToBuffer(
                &materialUBO,
                MATERIAL_BUFFER_SIZE,
                MATERIAL_BUFFER_SIZE * materialIndex);
            material->_clearDirty();
        }
    }

    void SceneRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        setInitialState(commandBuffer, currentFrame);
        if (ModelsRenderer::frameData[currentFrame].currentCamera == nullptr)
            return;
        if (!ModelsRenderer::frameData[currentFrame].models.empty()) {
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBiasEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBias(commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
            bindDescriptorSets(commandBuffer, currentFrame);
            drawModels(commandBuffer, currentFrame, frameData[currentFrame].opaquesModels);
        }
        if (frameData[currentFrame].skyboxRenderer != nullptr)
            frameData[currentFrame].skyboxRenderer->recordCommands(commandBuffer, currentFrame);
    }

    void SceneRenderer::createDescriptorSetLayout() {
        descriptorPool =
                DescriptorPool::Builder(device)
                        .setMaxSets(device.getFramesInFlight())
                        // global, models, materials+textures, shadow maps & lights UBOs
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 6 * device.getFramesInFlight())
                        // textures, shadow maps, shadow cubemap & PBR*3
                        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6 * device.getFramesInFlight())
                        .build();

        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(BINDING_GLOBAL_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_ALL_GRAPHICS)
                            .addBinding(BINDING_MODELS_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_VERTEX_BIT)
                            .addBinding(BINDING_MATERIALS_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_ALL_GRAPHICS)
                            .addBinding(BINDING_TEXTURES_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(BINDING_TEXTURES,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        MAX_IMAGES)
                            .addBinding(BINDING_LIGHTS_BUFFER,
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(BINDING_SHADOW_MAPS,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        MAX_SHADOW_MAPS)
                            .addBinding(BINDING_SHADOW_CUBEMAPS,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        MAX_SHADOW_MAPS)
                            .addBinding(BINDING_PBR_ENV_MAP,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(BINDING_PBR_IRRADIANCE_MAP,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .addBinding(BINDING_PBR_BRDF_LUT,
                                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                        VK_SHADER_STAGE_FRAGMENT_BIT)
                            .build();

        // Create an in-memory default blank images
        if (blankImage == nullptr) { blankImage = reinterpret_pointer_cast<VulkanImage>(Image::createBlankImage()); }
        if (blankCubemap == nullptr) { blankCubemap = reinterpret_pointer_cast<VulkanCubemap>(Cubemap::createBlankCubemap()); }

        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            frameData[i].globalBuffer = createUniformBuffer(GLOBAL_BUFFER_SIZE);
        }
    }

    void SceneRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto frameIndex = 0; frameIndex < device.getFramesInFlight(); frameIndex++) {
            auto& frame = frameData[frameIndex];
            if (!ModelsRenderer::frameData[frameIndex].models.empty() && (frame.modelBufferCount != ModelsRenderer::frameData[frameIndex].models.size())) {
                frame.modelBufferCount = ModelsRenderer::frameData[frameIndex].models.size();
                ModelsRenderer::frameData[frameIndex].modelUniformBuffer = createUniformBuffer(MODEL_BUFFER_SIZE * frame.modelBufferCount);
            }
            if (frame.modelBufferCount == 0) {
                frame.modelBufferCount = 1;
                ModelsRenderer::frameData[frameIndex].modelUniformBuffer = createUniformBuffer(MODEL_BUFFER_SIZE);
            }
            if (frame.materialsBuffer == nullptr) {
                frame.materialsBuffer = createUniformBuffer(MATERIAL_BUFFER_SIZE * MAX_MATERIALS);
                frame.texturesBuffer = createUniformBuffer(TEXTURE_BUFFER_SIZE * MAX_MATERIALS);
                // Initialize all materials & textures buffers in GPU memory
                auto materialUBOArray = make_unique<MaterialBuffer[]>(MAX_MATERIALS);
                writeUniformBuffer(frame.materialsBuffer, materialUBOArray.get());
                auto textureUBOArray = make_unique<TextureBuffer[]>(MAX_MATERIALS);
                writeUniformBuffer(frame.texturesBuffer, textureUBOArray.get());
            }

            if ((!frame.lights.empty()) && (frame.lightBufferCount != frame.lights.size())) {
                frame.lightBufferCount = frame.lights.size();
                frame.lightBuffer = createUniformBuffer(LIGHT_BUFFER_SIZE * frame.lightBufferCount);
            }
            if (frame.lightBufferCount == 0) {
                frame.lightBufferCount = 1;
                frame.lightBuffer = createUniformBuffer(LIGHT_BUFFER_SIZE);
            }

            uint32_t imageIndex = 0;
            for (const auto &image : frame.images) {
                frame.imagesInfo[imageIndex] = image->getImageInfo();
                imageIndex += 1;
            }
            for (auto j = imageIndex; j < frame.imagesInfo.size(); j++) {
                frame.imagesInfo[j] = blankImage->getImageInfo();
            }

            for (auto j = 0; j < frame.shadowMapsInfo.size(); j++) {
                frame.shadowMapsInfo[j] = blankImage->getImageInfo();
                frame.shadowMapsCubemapInfo[j] = blankCubemap->getImageInfo();
            }
            imageIndex = 0;
            for (const auto &pair : shadowMapRenderers) {
                const auto &shadowMap      = pair.second->getShadowMap(frameIndex);
                shadowMap->_setBufferIndex(imageIndex);
                const auto shadowMapInfo = VkDescriptorImageInfo{
                    .sampler     = shadowMap->getSampler(),
                    .imageView   = shadowMap->getImageView(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                };
                if (pair.second->isCubemap()) {
                    frame.shadowMapsCubemapInfo[imageIndex] = shadowMapInfo;
                } else {
                    frame.shadowMapsInfo[imageIndex] = shadowMapInfo;
                }
                imageIndex += 1;
            }

            auto globalBufferInfo     = frame.globalBuffer->descriptorInfo(GLOBAL_BUFFER_SIZE);
            auto modelBufferInfo      = ModelsRenderer::frameData[frameIndex].modelUniformBuffer->descriptorInfo(
                 MODEL_BUFFER_SIZE *
                 frame.modelBufferCount);
            auto materialBufferInfo   = frame.materialsBuffer->descriptorInfo(MATERIAL_BUFFER_SIZE * MAX_MATERIALS);
            auto textureBufferInfo   = frame.texturesBuffer->descriptorInfo(TEXTURE_BUFFER_SIZE * MAX_MATERIALS);
            auto pointLightBufferInfo = frame.lightBuffer->descriptorInfo(LIGHT_BUFFER_SIZE * frame.lightBufferCount);

            VkDescriptorImageInfo specularInfo;
            VkDescriptorImageInfo irradianceInfo;
            VkDescriptorImageInfo brdfInfo;
            if ((frame.skyboxRenderer != nullptr) && (frame.skyboxRenderer->getCubemap()->getCubemapType() == Cubemap::TYPE_ENVIRONMENT)) {
                const auto& environmentCubemap = reinterpret_pointer_cast<EnvironmentCubemap>(frame.skyboxRenderer->getCubemap());
                specularInfo = reinterpret_pointer_cast<VulkanCubemap>(environmentCubemap->getSpecularCubemap())->getImageInfo();
                irradianceInfo = reinterpret_pointer_cast<VulkanCubemap>(environmentCubemap->getIrradianceCubemap())->getImageInfo();
                brdfInfo = reinterpret_pointer_cast<VulkanImage>(environmentCubemap->getBRDFLut())->getImageInfo();
            } else {
                specularInfo = blankCubemap->getImageInfo();
                irradianceInfo = blankCubemap->getImageInfo();
                brdfInfo = blankImage->getImageInfo();
            }
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                .writeBuffer(BINDING_GLOBAL_BUFFER, &globalBufferInfo)
                .writeBuffer(BINDING_MODELS_BUFFER, &modelBufferInfo)
                .writeBuffer(BINDING_MATERIALS_BUFFER, &materialBufferInfo)
                .writeBuffer(BINDING_TEXTURES_BUFFER, &textureBufferInfo)
                .writeBuffer(BINDING_LIGHTS_BUFFER, &pointLightBufferInfo)
                .writeImage(BINDING_TEXTURES, frame.imagesInfo.data())
                .writeImage(BINDING_SHADOW_MAPS, frame.shadowMapsInfo.data())
                .writeImage(BINDING_SHADOW_CUBEMAPS, frame.shadowMapsCubemapInfo.data())
                .writeImage(BINDING_PBR_ENV_MAP, &specularInfo)
                .writeImage(BINDING_PBR_IRRADIANCE_MAP, &irradianceInfo)
                .writeImage(BINDING_PBR_BRDF_LUT, &brdfInfo);
            if (!writer.build(descriptorSet[frameIndex], create))
                die("Cannot allocate descriptor set for scene renderer");
        }
    }

    void SceneRenderer::loadShaders() {
        for (auto& frame : frameData) {
            if (frame.skyboxRenderer != nullptr) {
                frame.skyboxRenderer->loadShaders();
            }
        }
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::createImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            colorFrameBufferHdr[i] = make_shared<ColorFrameBufferHDR>(device);
            if (ModelsRenderer::frameData[i].depthFrameBuffer == nullptr) {
                ModelsRenderer::frameData[i].depthFrameBuffer = make_shared<DepthFrameBuffer>(device, true);
                resolvedDepthFrameBuffer[i] = make_shared<DepthFrameBuffer>(device, false);
            } else {
                ModelsRenderer::frameData[i].depthFrameBuffer->createImagesResources();
            }
        }
    }

    void SceneRenderer::cleanupImagesResources() {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            if (ModelsRenderer::frameData[i].depthFrameBuffer != nullptr) {
                resolvedDepthFrameBuffer[i]->cleanupImagesResources();
            }
            colorFrameBufferHdr[i]->cleanupImagesResources();
            frameData[i].colorFrameBufferMultisampled->cleanupImagesResources();
        }
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            colorFrameBufferHdr[i]->createImagesResources();
             frameData[i].colorFrameBufferMultisampled->createImagesResources();
            if (ModelsRenderer::frameData[i].depthFrameBuffer != nullptr) {
                 resolvedDepthFrameBuffer[i]->createImagesResources();
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
                .resolveImageView   = resolvedDepthFrameBuffer[currentFrame]->getImageView(),
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
                                       resolvedDepthFrameBuffer[currentFrame]->getImage(),
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                                      0,
                                      VK_ACCESS_SHADER_READ_BIT,
                                      VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                      VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void SceneRenderer::addImage(const shared_ptr<Image> &image, const uint32_t currentFrame) {
        const auto& vkImage = reinterpret_pointer_cast<VulkanImage>(image);
        frameData[currentFrame].imagesRefCounter[vkImage->getId()]++;
        if (find(frameData[currentFrame].images.begin(), frameData[currentFrame].images.end(), vkImage) != frameData[currentFrame].images.end())
            return;
        if (frameData[currentFrame].images.size() == MAX_IMAGES)
            die("Maximum images count reached for the scene renderer");
        frameData[currentFrame].imagesIndices[vkImage->getId()] = static_cast<int32_t>(frameData[currentFrame].images.size());
        frameData[currentFrame].images.push_back(vkImage);
    }

    void SceneRenderer::removeImage(const shared_ptr<Image> &image, const uint32_t currentFrame) {
        const auto& vkImage = reinterpret_pointer_cast<VulkanImage>(image);
        if (find(frameData[currentFrame].images.begin(), frameData[currentFrame].images.end(), vkImage) != frameData[currentFrame].images.end()) {
            if (--frameData[currentFrame].imagesRefCounter[vkImage->getId()] == 0) {
                frameData[currentFrame].images.remove(vkImage);
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
                                   const list<shared_ptr<MeshInstance>> &modelsToDraw) {
        auto shadersChanged = false;
        const auto cameraFrustum = Frustum{
            ModelsRenderer::frameData[currentFrame].currentCamera,
            ModelsRenderer::frameData[currentFrame].currentCamera->getFov(),
            ModelsRenderer::frameData[currentFrame].currentCamera->getNearDistance(),
            ModelsRenderer::frameData[currentFrame].currentCamera->getFarDistance()
        };

        // Used to reduce vkCmdSetCullMode calls
        auto lastCullMode = CULLMODE_BACK;
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);

        // Used to reduce vkCmdBindVertexBuffers & vkCmdBindIndexBuffer calls
        auto lastMeshId = Resource::id_t{numeric_limits<uint32_t>::max()};

        for (const auto &meshInstance : modelsToDraw) {
            if (meshInstance->isValid() && cameraFrustum.isOnFrustum(meshInstance)) {
                const auto &modelIndex = frameData[currentFrame].modelsIndices[meshInstance->getId()];
                const auto &model      = reinterpret_pointer_cast<VulkanMesh>(meshInstance->getMesh());
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
                                            0, PUSHCONSTANTS_SIZE, &pushConstants);
                        if (lastMeshId == model->getId()) {
                            model->bindlessDraw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                        } else {
                            model->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
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
                                        0, PUSHCONSTANTS_SIZE, &pushConstants);
                    if (lastMeshId == model->getId()) {
                        model->bindlessDraw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    } else {
                        model->draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
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

    void SceneRenderer::enableLightShadowCasting(const shared_ptr<Light>&light) {
        if (enableShadowMapRenders) {
            if (!shadowMapRenderers.contains(light)) {
                if (light->getCastShadows() && (shadowMapRenderers.size() < MAX_SHADOW_MAPS)) {
                    const auto shadowMapRenderer = make_shared<ShadowMapRenderer>(device, shaderDirectory, light);
                    for(auto i = 0; i < device.getFramesInFlight(); i++) {
                        shadowMapRenderer->activateCamera(ModelsRenderer::frameData[0].currentCamera, i);
                    }
                    shadowMapRenderers[light] = shadowMapRenderer;
                    device.registerRenderer(shadowMapRenderer);
                }
            }
        }
    }

    void SceneRenderer::disableLightShadowCasting(const shared_ptr<Light>&light) {
        if (enableShadowMapRenders) {
            if (shadowMapRenderers.contains(light)) {
                device.unRegisterRenderer(shadowMapRenderers.at(light));
                shadowMapRenderers.erase(light);
            }
        }
    }

} // namespace z0