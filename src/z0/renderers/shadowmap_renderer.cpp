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
import :ColorFrameBufferHDR;
import :ShadowMapRenderer;

// #define SHADOWMAP_RENDERER_DEBUG 1

namespace z0 {

    ShadowMapRenderer::ShadowMapRenderer(Device &device, const string &shaderDirectory, const Light *light) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR},
        light{light},
        lightIsDirectional{dynamic_cast<const DirectionalLight *>(light) != nullptr},
        shadowMap{make_shared<ShadowMapFrameBuffer>(device, lightIsDirectional)} {
        createImagesResources();
    }

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
            const auto *directionalLight = dynamic_cast<const DirectionalLight *>(light);
            const auto lightDirection = normalize(mat3{directionalLight->getTransformGlobal()} * directionalLight->getDirection());

            // https://www.saschawillems.de/blog/2017/12/30/new-vulkan-example-cascaded-shadow-mapping/
            // https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
            float cascadeSplits[ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS];

            const auto nearClip  = currentCamera->getNearClipDistance();
            const auto farClip   = currentCamera->getFarClipDistance();
            const auto clipRange = farClip - nearClip;

            const auto minZ = nearClip;
            const auto maxZ = nearClip + clipRange;

            const auto range = maxZ - minZ;
            const auto ratio = maxZ / minZ;

            // Calculate split depths based on view camera frustum
            // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
            for (uint32_t i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
                float p          = (i + 1) / static_cast<float>(ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS);
                float log        = minZ * std::pow(ratio, p);
                float uniform    = minZ + range * p;
                float d          = cascadeSplitLambda * (log - uniform) + uniform;
                cascadeSplits[i] = (d - nearClip) / clipRange;
            }

            // Calculate orthographic projection matrix for each cascade
            float lastSplitDist = 0.0;
            const auto invCam = inverse(currentCamera->getProjection() * currentCamera->getView());
            for (uint32_t i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
                const auto splitDist = cascadeSplits[i];

                // Camera frustum corners in NDC space
                vec3 frustumCorners[8] = {
                        vec3(-1.0f, 1.0f, -1.0f),
                        vec3(1.0f, 1.0f, -1.0f),
                        vec3(1.0f, -1.0f, -1.0f),
                        vec3(-1.0f, -1.0f, -1.0f),

                        vec3(-1.0f, 1.0f, 1.0f),
                        vec3(1.0f, 1.0f, 1.0f),
                        vec3(1.0f, -1.0f, 1.0f),
                        vec3(-1.0f, -1.0f, 1.0f),
                };

                // Camera frustum corners into world space
                for (uint32_t j = 0; j < 8; j++) {
                    const auto invCorner = invCam * vec4(frustumCorners[j], 1.0f);
                    frustumCorners[j]   = invCorner / invCorner.w;
                }

                // Adjust the coordinates of near and far planes for this specific cascade
                for (uint32_t j = 0; j < 4; j++) {
                    const auto dist        = frustumCorners[j + 4] - frustumCorners[j];
                    frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                    frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
                }

                // Frustum center for this cascade split, in world space
                auto frustumCenter = VEC3ZERO;
                for (uint32_t j = 0; j < 8; j++) {
                    frustumCenter += frustumCorners[j];
                }
                frustumCenter /= 8.0f;

                // Radius of the cascade split
                auto radius = 0.0f;
                for (uint32_t j = 0; j < 8; j++) {
                    const auto distance = length(frustumCorners[j] - frustumCenter);
                    radius         = glm::max(radius, distance);
                }
                radius = std::ceil(radius * 16.0f) / 16.0f;

                auto maxExtents = vec3(radius);
                auto minExtents = -maxExtents;

                auto lightPosition  = frustumCenter - lightDirection * -minExtents.z;
                const auto depth = maxExtents.z - minExtents.z;
                const auto lightProjection = ortho(
                    minExtents.x, maxExtents.x,
                        minExtents.y, maxExtents.y,
                        -depth, depth);
                lightSpace[i] = lightProjection * lookAt(lightPosition, frustumCenter, AXIS_UP);
                splitDepth[i] = (currentCamera->getNearClipDistance() + splitDist * clipRange);
                lastSplitDist = cascadeSplits[i];
            }
        } else if (auto *spotLight = dynamic_cast<const SpotLight *>(light)) {
            const auto lightDirection = normalize(mat3{spotLight->getTransformGlobal()} * spotLight->getDirection());
            const auto lightPosition       = light->getPositionGlobal();
            const auto sceneCenter         = lightPosition + lightDirection;
            const auto lightProjection     = glm::perspective(spotLight->getFov(), device.getAspectRatio(), 0.1f, 20.0f);
            lightSpace[0] = lightProjection * lookAt(lightPosition, sceneCenter, -AXIS_UP);
        } else {
            lightSpace[0] = mat4{};
        }
    }

    void ShadowMapRenderer::update(const uint32_t currentFrame) {
        if (currentCamera == nullptr) { return; }
        updateLightSpace();
        if (isCascaded()) {
            for (int i = 0; i < ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS; i++) {
                const GobalUniformBuffer globalUbo{
                    .lightSpace = lightSpace[i],
                };
                writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo, i);
            }
        } else {
            const GobalUniformBuffer globalUbo{
                .lightSpace = lightSpace[0],
            };
            writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo, 0);
        }

        uint32_t modelIndex = 0;
        for (const auto &meshInstance : models) {
            ModelUniformBuffer modelUbo{
                    .matrix = meshInstance->getTransformGlobal()
            };
            writeUniformBuffer(modelUniformBuffers, currentFrame, &modelUbo, modelIndex);
            modelIndex += 1;
        }
    }

    void ShadowMapRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        const auto cascades = lightIsDirectional ? ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS : 1;
        for (int cascadeIndex = 0; cascadeIndex < cascades; cascadeIndex++) {
            device.transitionImageLayout(commandBuffer,
                                                shadowMap->getImage(),
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                0,
                                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                VK_IMAGE_ASPECT_DEPTH_BIT);
#ifdef SHADOWMAP_RENDERER_DEBUG
            const VkRenderingAttachmentInfo colorAttachmentInfo{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                                                           .imageView   = colorAttachmentHdr->getImageView(),
                                                           .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                           .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                           .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                                                           .clearValue  = clearColor};
#endif
            const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView   = shadowMap->getImageView(cascadeIndex),
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
#ifdef SHADOWMAP_RENDERER_DEBUG
                                                .colorAttachmentCount = 1,
                                                .pColorAttachments    =  &colorAttachmentInfo,
#else
                                                .colorAttachmentCount = 0,
                                                .pColorAttachments    =  nullptr,
#endif
                                                .pDepthAttachment     = &depthAttachmentInfo,
                                                .pStencilAttachment   = nullptr};
            vkCmdBeginRendering(commandBuffer, &renderingInfo);

            bindShaders(commandBuffer);
            setViewport(commandBuffer, shadowMap->getSize(), shadowMap->getSize());
            vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
            constexpr VkBool32 color_blend_enables[] = {VK_FALSE};
            vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
            vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);
            const auto vertexBinding   = Mesh::_getBindingDescription();
            const auto vertexAttribute = Mesh::_getAttributeDescription();
            vkCmdSetVertexInputEXT(commandBuffer,
                                   vertexBinding.size(),
                                   vertexBinding.data(),
                                   vertexAttribute.size(),
                                   vertexAttribute.data());
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBiasEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBias(commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);

            uint32_t modelIndex = 0;
            for (const auto &meshInstance : models) {
                if (meshInstance->isValid()) {
                    auto mesh = meshInstance->getMesh();
                    for (const auto &surface : mesh->getSurfaces()) {
                        array<uint32_t, 2> offsets = {
                            static_cast<uint32_t>(globalUniformBuffers[currentFrame]->getAlignmentSize() *
                                              cascadeIndex),
                            static_cast<uint32_t>(modelUniformBuffers[currentFrame]->getAlignmentSize() *
                                                  modelIndex),
                        };
                        bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                        mesh->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    }
                }
                modelIndex += 1;
            }
            vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);

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
#ifdef SHADOWMAP_RENDERER_DEBUG
            device.transitionImageLayout(commandBuffer,
                                   colorAttachmentHdr->getImage(),
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   0,
                                   VK_ACCESS_SHADER_READ_BIT ,
                                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                   VK_IMAGE_ASPECT_COLOR_BIT);
#endif
        }
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
        createUniformBuffers(
                globalUniformBuffers, globalUniformBufferSize, ShadowMapFrameBuffer::CASCADED_SHADOWMAP_LAYERS);
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
#ifdef SHADOWMAP_RENDERER_DEBUG
        fragShader = createShader("shadowmap.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0); // for debug
#endif
    }

    void ShadowMapRenderer::createImagesResources() {
#ifdef SHADOWMAP_RENDERER_DEBUG
        colorAttachmentHdr = make_shared<ColorFrameBufferHDR>(device, shadowMap->getSize(), shadowMap->getSize());
#endif
    }

    void ShadowMapRenderer::cleanupImagesResources() {
        if (shadowMap != nullptr) {
            shadowMap->cleanupImagesResources();
#ifdef SHADOWMAP_RENDERER_DEBUG
            colorAttachmentHdr->cleanupImagesResources();
#endif
        }
    }

} // namespace z0
