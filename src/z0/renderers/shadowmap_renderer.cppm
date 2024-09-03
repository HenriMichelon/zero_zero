module;
#include "z0/libraries.h"
#include <volk.h>

export module Z0:ShadowMapRenderer;

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

export namespace z0 {
    /**
     * Shadow map renderer, one per light
     */
    class ShadowMapRenderer : public Renderpass, public Renderer {
    public:
        ShadowMapRenderer(Device& device, const string& shaderDirectory, Light* light, const vec3 position):
            Renderpass{device, shaderDirectory},
            shadowMap{make_shared<ShadowMapFrameBuffer>(device, light, position)} {
        }

        void loadScene(const list<MeshInstance*>& meshes) {
            models = meshes;
            createOrUpdateResources();
        }

        void cleanup() override {
            cleanupImagesResources();
            shadowMap.reset();
            modelUniformBuffers.clear();
            Renderpass::cleanup();
        }

        [[nodiscard]] const shared_ptr<ShadowMapFrameBuffer>& getShadowMap() const { return shadowMap; };

        ~ShadowMapRenderer() override {
            ShadowMapRenderer::cleanup();
        }

    private:
        struct GobalUniformBuffer {
            mat4 lightSpace;
        };

        struct ModelUniformBuffer {
            mat4 matrix;
        };

        // Depth bias (and slope) are used to avoid shadowing artifacts
        // Constant depth bias factor (always applied)
        const float depthBiasConstant = 1.25f;
        // Slope depth bias factor, applied depending on polygon's slope
        const float depthBiasSlope = 1.75f;
        // All the models of the scene
        list<MeshInstance*> models{};
        // Datas for all the models of the scene, one buffer for all the models
        // https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        // Size of the model uniform buffer
        static constexpr VkDeviceSize modelUniformBufferSize{sizeof(ModelUniformBuffer)};
        // Currently allocated model uniform buffer count
        uint32_t modelUniformBufferCount{0};
        shared_ptr<ShadowMapFrameBuffer> shadowMap;

        void update(const uint32_t currentFrame) override {
            const GobalUniformBuffer globalUbo{
                .lightSpace = shadowMap->getLightSpace()
            };
            writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);

            uint32_t modelIndex = 0;
            for (const auto& meshInstance : models) {
                ModelUniformBuffer modelUbo{
                    .matrix = meshInstance->getTransformGlobal(),
                };
                writeUniformBuffer(modelUniformBuffers, currentFrame, &modelUbo, modelIndex);
                modelIndex += 1;
            }
        }

        void recordCommands(VkCommandBuffer commandBuffer, const uint32_t currentFrame) override {
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
            setViewport(commandBuffer, shadowMap->size, shadowMap->size);

            const auto vertexBinding = Mesh::_getBindingDescription();
            const auto vertexAttribute = Mesh::_getAttributeDescription();
            vkCmdSetVertexInputEXT(commandBuffer,
                                   vertexBinding.size(),
                                   vertexBinding.data(),
                                   vertexAttribute.size(),
                                   vertexAttribute.data());

            uint32_t modelIndex = 0;
            for (const auto& meshInstance : models) {
                if (meshInstance->isValid()) {
                    auto mesh = meshInstance->getMesh();
                    for (const auto& surface : mesh->getSurfaces()) {
                        /*if (auto standardMaterial = dynamic_cast<StandardMaterial*>(surface->material.get())) {
                             vkCmdSetCullMode(commandBuffer,
                                         surface->material->getCullMode() == CULLMODE_DISABLED ? VK_CULL_MODE_NONE :
                                         surface->material->getCullMode() == CULLMODE_BACK ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT);
                        } else {
                            //vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT); // default avoid Peter panning
                            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                        }*/
                        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_FRONT_BIT); // default avoid Peter panning
                        array<uint32_t, 2> offsets = {
                            0, // globalBuffers
                            static_cast<uint32_t>(modelUniformBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                        };
                        bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                        mesh->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                    }
                }
                modelIndex += 1;
            }
            vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
        }

        void createDescriptorSetLayout() override {
            descriptorPool = DescriptorPool::Builder(device)
                             .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // model UBO
                             .build();

            setLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0, // global UBO
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                    VK_SHADER_STAGE_VERTEX_BIT)
                        .addBinding(1, // model UBO
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                    VK_SHADER_STAGE_VERTEX_BIT)
                        .build();

            globalUniformBufferSize = sizeof(GobalUniformBuffer);
            createUniformBuffers(globalUniformBuffers, globalUniformBufferSize);
        }

        void createOrUpdateDescriptorSet(bool create) override {
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
                auto modelBufferInfo = modelUniformBuffers[i]->descriptorInfo(modelUniformBufferSize);
                if (!DescriptorWriter(*setLayout, *descriptorPool)
                     .writeBuffer(0, &globalBufferInfo)
                     .writeBuffer(1, &modelBufferInfo)
                     .build(descriptorSet[i])) {
                    die("Cannot allocate descriptor set for shadow maps");
                }
            }
        }

        void loadShaders() override {
            vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
        }

        void createImagesResources() override {
            shadowMap->createImagesResources();
        }

        void cleanupImagesResources() override {
            if (shadowMap != nullptr) {
                shadowMap->cleanupImagesResources();
            }
        }

        void recreateImagesResources() override {
        }

        void beginRendering(VkCommandBuffer commandBuffer) override {
            device.transitionImageLayout(commandBuffer, shadowMap->getImage(),
                                         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                         0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                         VK_IMAGE_ASPECT_DEPTH_BIT);
            const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = shadowMap->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = depthClearValue,
            };
            const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {
                    {0, 0},
                    {shadowMap->size, shadowMap->size}
                },
                .layerCount = 1,
                .colorAttachmentCount = 0,
                .pColorAttachments = nullptr,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
            };
            vkCmdBeginRendering(commandBuffer, &renderingInfo);
        }

        void endRendering(VkCommandBuffer commandBuffer, const bool isLast) override {
            vkCmdEndRendering(commandBuffer);
            device.transitionImageLayout(
                commandBuffer, shadowMap->getImage(),
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT, // After depth writes
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Before depth reads in the shader
                VK_IMAGE_ASPECT_DEPTH_BIT);
        }

    public:
        ShadowMapRenderer(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer& operator=(const ShadowMapRenderer&) = delete;
        ShadowMapRenderer(const ShadowMapRenderer&&) = delete;
        ShadowMapRenderer&& operator=(const ShadowMapRenderer&&) = delete;
    };

}
