module;
#include "z0/libraries.h"
#include <volk.h>

export module Z0:SkyboxRenderer;

import :Tools;
import :Renderpass;
import :Device;
import :Cubemap;
import :Environment;
import :Camera;
import :Buffer;
import :Descriptors;

export namespace z0 {

    class SkyboxRenderer : public Renderpass {
    public:
        SkyboxRenderer(Device& device, const string& shaderDirectory):
            Renderpass{device, shaderDirectory} {
            static constexpr float skyboxVertices[] = {
                // positions
                -1.0f, 1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,

                -1.0f, -1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,

                -1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, 1.0f, -1.0f,

                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f
            };
            vertexCount = 108 / 3;
            uint32_t vertexSize = sizeof(float) * 3;
            const VkDeviceSize bufferSize = vertexSize * vertexCount;
            const Buffer stagingBuffer{
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            };
            stagingBuffer.writeToBuffer((void*)skyboxVertices);
            vertexBuffer = make_unique<Buffer>(
                device,
                vertexSize,
                vertexCount,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
            );
            stagingBuffer.copyTo(*vertexBuffer, bufferSize);
        }

        void loadScene(const shared_ptr<Cubemap>& _cubemap) {
            cubemap = _cubemap;
            createOrUpdateResources();
        }

        void cleanup() override {
            vertexBuffer.reset();
            cubemap.reset();
            Renderpass::cleanup();
        }

        void update(Camera* currentCamera, const Environment* currentEnvironment, const uint32_t currentFrame) {
            GobalUniformBuffer globalUbo{
                .projection = currentCamera->getProjection(),
                .view = mat4(mat3(currentCamera->getView()))
            };
            if (currentEnvironment != nullptr) {
                globalUbo.ambient = currentEnvironment->getAmbientColorAndIntensity();
            }
            writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);
        }

        void loadShaders() override {
            vertShader = createShader("skybox.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
            fragShader = createShader("skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        }

        void createDescriptorSetLayout() override {
            descriptorPool = DescriptorPool::Builder(device)
                             .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                             .build();

            globalUniformBufferSize = sizeof(GobalUniformBuffer);
            createUniformBuffers(globalUniformBuffers, globalUniformBufferSize);

            setLayout = DescriptorSetLayout::Builder(device)
                        .addBinding(0,
                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                    VK_SHADER_STAGE_VERTEX_BIT)
                        .addBinding(1,
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                    VK_SHADER_STAGE_FRAGMENT_BIT,
                                    1)
                        .build();
        }

        void createOrUpdateDescriptorSet(const bool create) override {
            if (create) {
                for (uint32_t i = 0; i < descriptorSet.size(); i++) {
                    auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
                    auto imageInfo = cubemap->_getImageInfo();
                    auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                                  .writeBuffer(0, &globalBufferInfo)
                                  .writeImage(1, &imageInfo);
                    if (!writer.build(descriptorSet[i])) die("Cannot allocate skybox renderer descriptor set");
                }
            }
        }

        void recordCommands(VkCommandBuffer commandBuffer, const uint32_t currentFrame) override {
            bindShaders(commandBuffer);
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);
            vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
            VkVertexInputBindingDescription2EXT bindingDescription{
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
                .binding = 0,
                .stride = 3 * sizeof(float),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                .divisor = 1,
            };
            VkVertexInputAttributeDescription2EXT attributeDescription{
                VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                nullptr,
                0,
                0,
                VK_FORMAT_R32G32B32_SFLOAT,
                0
            };
            vkCmdSetVertexInputEXT(commandBuffer,
                                   1,
                                   &bindingDescription,
                                   1,
                                   &attributeDescription);
            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_BACK_BIT);
            bindDescriptorSets(commandBuffer, currentFrame);
            const VkBuffer buffers[] = {vertexBuffer->getBuffer()};
            constexpr VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }

    private:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
        };

        uint32_t vertexCount;
        shared_ptr<Cubemap> cubemap;
        unique_ptr<Buffer> vertexBuffer;
    };

}
