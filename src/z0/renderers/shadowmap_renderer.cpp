module;
#include <cstdlib>
#include <volk.h>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Tools;
import :Renderer;
import :Renderpass;
import :Device;
import :ShadowMapFrameBuffer;
import :Descriptors;
import :MeshInstance;
import :Light;
import :Buffer;
import :Mesh;
import :ShadowMapRenderer;

namespace z0 {

    ShadowMapRenderer::ShadowMapRenderer(Device &device, const string &shaderDirectory, const Light *light) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR},
        light{light},
        lightIsDirectional{dynamic_cast<const DirectionalLight *>(light) != nullptr},
        cascaded{lightIsDirectional},
        shadowMap{make_shared<ShadowMapFrameBuffer>(device, cascaded)} {}

    void ShadowMapRenderer::loadScene(const list<MeshInstance *> &meshes) {
        models = meshes;
        createOrUpdateResources();
    }

    void ShadowMapRenderer::cleanup() {
        cleanupImagesResources();
        shadowMap.reset();
        modelUniformBuffers.clear();
        Renderpass::cleanup();
    }

    ShadowMapRenderer::~ShadowMapRenderer() { ShadowMapRenderer::cleanup(); }

    void ShadowMapRenderer::updateLightSpace() {
        if (lightIsDirectional) {
            // https://www.saschawillems.de/blog/2017/12/30/new-vulkan-example-cascaded-shadow-mapping/
            // float cascadeSplits[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS];
            //
            // const auto nearClip = currentCamera->getNearClipDistance();
            // const auto farClip = currentCamera->getFarClipDistance();
            // const auto clipRange = farClip - nearClip;
            //
            // const auto minZ = nearClip;
            // const auto maxZ = nearClip + clipRange;
            //
            // const auto range = maxZ - minZ;
            // const auto ratio = maxZ / minZ;
            //
            // // Calculate split depths based on view camera frustum
            // // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
            // for (uint32_t i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
            //     float p = (i + 1) / static_cast<float>(ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS);
            //     float log = minZ * std::pow(ratio, p);
            //     float uniform = minZ + range * p;
            //     float d = cascadeSplitLambda * (log - uniform) + uniform;
            //     cascadeSplits[i] = (d - nearClip) / clipRange;
            // }
            //
            // // Calculate orthographic projection matrix for each cascade
            // float lastSplitDist = 0.0;
            // for (uint32_t i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
            //     float splitDist = cascadeSplits[i];
            //
            //     glm::vec3 frustumCorners[8] = {
            //         glm::vec3(-1.0f,  1.0f, 0.0f),
            //         glm::vec3( 1.0f,  1.0f, 0.0f),
            //         glm::vec3( 1.0f, -1.0f, 0.0f),
            //         glm::vec3(-1.0f, -1.0f, 0.0f),
            //         glm::vec3(-1.0f,  1.0f,  1.0f),
            //         glm::vec3( 1.0f,  1.0f,  1.0f),
            //         glm::vec3( 1.0f, -1.0f,  1.0f),
            //         glm::vec3(-1.0f, -1.0f,  1.0f),
            //     };
            //
            //     // Project frustum corners into world space
            //     glm::mat4 invCam = glm::inverse(currentCamera->getProjection() * currentCamera->getView());
            //     for (uint32_t j = 0; j < 8; j++) {
            //         glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
            //         frustumCorners[j] = invCorner / invCorner.w;
            //     }
            //
            //     for (uint32_t j = 0; j < 4; j++) {
            //         glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
            //         frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
            //         frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
            //     }
            //
            //     // Get frustum center
            //     glm::vec3 frustumCenter = glm::vec3(0.0f);
            //     for (uint32_t j = 0; j < 8; j++) {
            //         frustumCenter += frustumCorners[j];
            //     }
            //     frustumCenter /= 8.0f;
            //
            //     float radius = 0.0f;
            //     for (uint32_t j = 0; j < 8; j++) {
            //         float distance = glm::length(frustumCorners[j] - frustumCenter);
            //         radius = glm::max(radius, distance);
            //     }
            //     radius = std::ceil(radius * 16.0f) / 16.0f;
            //
            //     glm::vec3 maxExtents = glm::vec3(radius);
            //     glm::vec3 minExtents = -maxExtents;
            //
            //     glm::vec3 lightDir = normalize(-light->getPositionGlobal());
            //     glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
            //     glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
            //
            //     // Store split distance and matrix in cascade
            //     splitDepth[i] = (currentCamera->getNearClipDistance() + splitDist * clipRange) * -1.0f;
            //     // if (i == 0) {
            //     //     log(to_string(currentCamera->getPositionGlobal()));
            //     //     log(to_string(frustumCorners[0]));
            //     //     log(to_string(splitDepth[i]));
            //     // }
            //     lightSpace[i] = lightOrthoMatrix * lightViewMatrix;
            //
            //     lastSplitDist = cascadeSplits[i];
            // }

            // const auto *directionalLight = dynamic_cast<const DirectionalLight *>(light);
            // const auto  lightDirection =
            //         normalize(mat3{directionalLight->getTransformGlobal()} * directionalLight->getDirection());
            // // Scene bounds
            // const auto limit            = 10.0f * (cascadeLayer + 1.0f);
            // auto       cameraPosition   = currentCamera->getPositionGlobal();
            // const vec3 worldTranslation = toQuat(mat3(currentCamera->getTransformGlobal())) * (AXIS_FRONT * limit);
            // cameraPosition += worldTranslation;
            // auto sceneMin = vec3{-limit, -limit, -limit} + cameraPosition;
            // auto sceneMax = vec3{limit, limit, limit} + cameraPosition;
            // // Set up the orthographic projection matrix
            // auto orthoWidth  = distance(sceneMin.x, sceneMax.x);
            // auto orthoHeight = distance(sceneMin.y, sceneMax.y);
            // auto orthoDepth  = distance(sceneMin.z, sceneMax.z);
            // sceneCenter      = (sceneMin + sceneMax) / 2.0f;
            // lightPosition    = sceneCenter - lightDirection * (orthoDepth / 2.0f);
            // // Position is scene center offset by light direction
            // lightProjection = ortho(
            //         -orthoWidth / 2, orthoWidth / 2, -orthoHeight / 2, orthoHeight / 2, -orthoDepth*2, orthoDepth);
        } else if (auto *spotLight = dynamic_cast<const SpotLight *>(light)) {
            const auto lightDirection = normalize(mat3{spotLight->getTransformGlobal()} * spotLight->getDirection());
            const auto lightPosition             = light->getPositionGlobal();
            const auto sceneCenter               = lightPosition + lightDirection;
            const auto lightProjection           = perspective(spotLight->getFov(), device.getAspectRatio(), 0.1f, 20.0f);
            // Combine the projection and view matrix to form the light's space matrix
            lightSpace[0] = lightProjection * lookAt(lightPosition, sceneCenter, AXIS_UP);
        } else {
            lightSpace[0] = mat4{};
        }
    }

    void ShadowMapRenderer::update(const uint32_t currentFrame) {
        if (currentCamera == nullptr) {
            return;
        }
        updateLightSpace();
        for (int i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
            const GobalUniformBuffer globalUbo {
                .lightSpace = lightSpace[i],
            };
            writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo, i);
        }

        uint32_t modelIndex = 0;
        for (const auto &meshInstance : models) {
            ModelUniformBuffer modelUbo{
                    .matrix = meshInstance->getTransformGlobal(),
            };
            writeUniformBuffer(modelUniformBuffers, currentFrame, &modelUbo, modelIndex);
            modelIndex += 1;
        }
    }

    void ShadowMapRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        bindShaders(commandBuffer);

        constexpr VkBool32 color_blend_enables[] = {VK_FALSE};
        vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
        vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);

        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthBiasEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS);
        vkCmdSetDepthBias(commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
        setViewport(commandBuffer, shadowMap->getSize(), shadowMap->getSize());

        const auto vertexBinding   = Mesh::_getBindingDescription();
        const auto vertexAttribute = Mesh::_getAttributeDescription();
        vkCmdSetVertexInputEXT(commandBuffer,
                               vertexBinding.size(),
                               vertexBinding.data(),
                               vertexAttribute.size(),
                               vertexAttribute.data());

        const auto cascadeCount = 1; //cascaded ? ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS : 1;
        for (uint32_t cascadeIndex = 0; cascadeIndex < cascadeCount; cascadeIndex++) {
            uint32_t modelIndex = 0;
            for (const auto &meshInstance : models) {
                if (meshInstance->isValid()) {
                    auto mesh = meshInstance->getMesh();
                    for (const auto &surface : mesh->getSurfaces()) {
                        /*if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                             vkCmdSetCullMode(commandBuffer,
                                         surface->material->getCullMode() == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         surface->material->getCullMode() == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT :
                        VK_CULL_MODE_FRONT_BIT); } else {
                            //vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT); // default avoid Peter panning
                            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                        }*/
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT); // default avoid Peter panning
                        array<uint32_t, 2> offsets = {
                            cascadeIndex,
                            static_cast<uint32_t>(modelUniformBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                        };
                        bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                        mesh->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    }
                }
                modelIndex += 1;
            }
        }
        vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
    }

    void ShadowMapRenderer::createDescriptorSetLayout() {
        descriptorPool =
                DescriptorPool::Builder(device)
                        .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                        .build();

        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0,
                                        // global UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                        VK_SHADER_STAGE_VERTEX_BIT)
                            .addBinding(1,
                                        // model UBO
                                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                        VK_SHADER_STAGE_VERTEX_BIT)
                            .build();

        globalUniformBufferSize = sizeof(GobalUniformBuffer);
        createUniformBuffers(globalUniformBuffers, globalUniformBufferSize, ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS);
    }

    void ShadowMapRenderer::createOrUpdateDescriptorSet(const bool create) {
        if (!models.empty() && (modelUniformBufferCount != models.size())) {
            modelUniformBufferCount = models.size();
            createUniformBuffers(modelUniformBuffers, modelUniformBufferSize, modelUniformBufferCount);
        }
        if (modelUniformBufferCount == 0) {
            modelUniformBufferCount = 1;
            createUniformBuffers(modelUniformBuffers, modelUniformBufferSize, modelUniformBufferCount);
        }
        for (uint32_t i = 0; i < descriptorSet.size(); i++) {
            auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto modelBufferInfo  = modelUniformBuffers[i]->descriptorInfo(modelUniformBufferSize);
            if (!DescriptorWriter(*setLayout, *descriptorPool)
                         .writeBuffer(0, &globalBufferInfo)
                         .writeBuffer(1, &modelBufferInfo)
                         .build(descriptorSet[i])) {
                die("Cannot allocate descriptor set for shadow maps");
            }
        }
    }

    void ShadowMapRenderer::loadShaders() {
        vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
    }

    void ShadowMapRenderer::createImagesResources() { shadowMap->createImagesResources(); }

    void ShadowMapRenderer::cleanupImagesResources() {
        if (shadowMap != nullptr) {
            shadowMap->cleanupImagesResources();
        }
    }

    void ShadowMapRenderer::recreateImagesResources() {}

    void ShadowMapRenderer::beginRendering(const VkCommandBuffer commandBuffer) {
        device.transitionImageLayout(commandBuffer,
                                     shadowMap->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                     0,
                                     VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                     VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                     VK_IMAGE_ASPECT_DEPTH_BIT);
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView   = shadowMap->getImageView(0),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue  = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{.sType      = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                                            .pNext      = nullptr,
                                            .renderArea = {{0, 0}, {shadowMap->getSize(), shadowMap->getSize()}},
                                            .layerCount = 1,
                                            .colorAttachmentCount = 0,
                                            .pColorAttachments    = nullptr,
                                            .pDepthAttachment     = &depthAttachmentInfo,
                                            .pStencilAttachment   = nullptr};
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void ShadowMapRenderer::endRendering(const VkCommandBuffer commandBuffer, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        device.transitionImageLayout(commandBuffer,
                                     shadowMap->getImage(),
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_ACCESS_SHADER_READ_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     // After depth writes
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     // Before depth reads in the shader
                                     VK_IMAGE_ASPECT_DEPTH_BIT);
    }

} // namespace z0
