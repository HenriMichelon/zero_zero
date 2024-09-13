module;
#include "z0/libraries.h"
#include <volk.h>

export module Z0:Renderpass;

import :Tools;
import :Constants;
import :Device;
import :Shader;
import :Buffer;
import :Descriptors;

export namespace z0 {

    /**
     * Base class for renderers and sub passes
     */
    class Renderpass {
    public:
        inline virtual ~Renderpass() = default;

        virtual void cleanup() {
            globalUniformBuffers.clear();
            if (vertShader != nullptr) vertShader.reset();
            if (fragShader != nullptr) fragShader.reset();
            if (pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(vkDevice, pipelineLayout, nullptr);
                pipelineLayout = VK_NULL_HANDLE;
            }
            setLayout.reset();
            descriptorPool.reset();
        }

        [[nodiscard]] const Device& getDevice() const { return device; }

    protected:
        Device& device;
        VkDevice vkDevice;
        string shaderDirectory; // SPIR-V Compiled shaders directory
        VkPipelineLayout pipelineLayout { VK_NULL_HANDLE };
        shared_ptr<DescriptorPool> descriptorPool {};
        unique_ptr<DescriptorSetLayout> setLayout {};
        vector<VkDescriptorSet> descriptorSet{MAX_FRAMES_IN_FLIGHT};
        unique_ptr<Shader> vertShader;
        unique_ptr<Shader> fragShader;
        VkDeviceSize globalUniformBufferSize{0};
        vector<unique_ptr<Buffer>> globalUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        bool descriptorSetNeedUpdate{false};

        const VkClearValue clearColor {{{
                    static_cast<float>(WINDOW_CLEAR_COLOR[0]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[1]) / 256.0f,
                    static_cast<float>(WINDOW_CLEAR_COLOR[2]) / 256.0f,
                    1.0f}}};
        const VkClearValue depthClearValue { .depthStencil = {1.0f, 0} };

        Renderpass(Device& dev, string  shaderDir) :
            device{dev},
            vkDevice{dev.getDevice()},
            shaderDirectory(std::move(shaderDir)) {}

        // Helpers function for children classes
        static void setViewport(VkCommandBuffer commandBuffer,
                                const uint32_t width,
                                const uint32_t height) {
            const VkExtent2D extent = { width, height };
            const VkViewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(extent.width),
                .height = static_cast<float>(extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            };
            vkCmdSetViewportWithCount(commandBuffer, 1, &viewport);
            const VkRect2D scissor{
                .offset = {0, 0},
                .extent = extent
            };
            vkCmdSetScissorWithCount(commandBuffer, 1, &scissor);
        }

        static void writeUniformBuffer(const vector<unique_ptr<Buffer>>& buffers,
                                        const uint32_t currentFrame,
                                        const void *data,
                                        const uint32_t index) {
            const auto size = buffers[currentFrame]->getAlignmentSize();
            buffers[currentFrame]->writeToBuffer(data, size, size * index);
        }

        static void writeUniformBuffer(const vector<unique_ptr<Buffer>>& buffers,
                                        const uint32_t currentFrame,
                                        const void *data) {
            buffers[currentFrame]->writeToBuffer(data);
        }

        void createOrUpdateResources() {
            if (descriptorPool == nullptr) {
                descriptorSetNeedUpdate = false;
                createDescriptorSetLayout();
                createOrUpdateDescriptorSet(true);
                if (setLayout != nullptr) {
                    createPipelineLayout();
                    loadShaders();
                }
            } else if (descriptorSetNeedUpdate) {
                descriptorSetNeedUpdate = false;
                createOrUpdateDescriptorSet(false);
            }
        }

        void createUniformBuffers(vector<unique_ptr<Buffer>>& buffers,
                                    const VkDeviceSize size,
                                    const uint32_t count = 1) const {
            for (auto &buffer: buffers) {
                buffer = make_unique<Buffer>(
                        device,
                        size,
                        count,
                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        device.getDeviceProperties().limits.minUniformBufferOffsetAlignment
                );
                if (buffer->map() != VK_SUCCESS) { die("Error mapping UBO to GPU memory"); }
            }
        }

        void bindDescriptorSets(VkCommandBuffer commandBuffer,
                                const uint32_t currentFrame,
                                const uint32_t count = 0,
                                const uint32_t *offsets = nullptr) const {
            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout,
                                    0, 1,
                                    &descriptorSet[currentFrame],
                                    count, offsets);
        }

        void bindShaders(VkCommandBuffer commandBuffer) const {
            if (vertShader != nullptr) {
                vkCmdBindShadersEXT(commandBuffer, 1, vertShader->getStage(), vertShader->getShader());
            } else {
                constexpr VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_VERTEX_BIT};
                vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
            }
            if (fragShader != nullptr) {
                vkCmdBindShadersEXT(commandBuffer, 1, fragShader->getStage(), fragShader->getShader());
            } else {
                constexpr VkShaderStageFlagBits stageFlagBits{VK_SHADER_STAGE_FRAGMENT_BIT};
                vkCmdBindShadersEXT(commandBuffer, 1, &stageFlagBits, VK_NULL_HANDLE);
            }
        }

        unique_ptr<Shader> createShader(const string& filename,
                                        const VkShaderStageFlagBits stage,
                                        const VkShaderStageFlags next_stage) {
            auto code = readFile(filename);
            auto shader  = make_unique<Shader>(
                    device,
                    stage,
                    next_stage,
                    filename,
                    code,
                    setLayout->getDescriptorSetLayout(),
                    nullptr);
            buildShader(*shader);
            return shader;
        }

        virtual void loadShaders() = 0;

        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

        virtual void createDescriptorSetLayout() = 0;

        virtual void createOrUpdateDescriptorSet(bool create) { };

        virtual void createPipelineLayout() {
            const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = setLayout->getDescriptorSetLayout(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr
            };
            if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                die("failed to create pipeline layout!");
            }
        }

    private:
        void buildShader(Shader& shader) const {
            // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
            VkShaderEXT shaderEXT;
            const VkShaderCreateInfoEXT shaderCreateInfo = shader.getShaderCreateInfo();
            if (vkCreateShadersEXT(vkDevice, 1, &shaderCreateInfo, nullptr, &shaderEXT) != VK_SUCCESS) {
                die("vkCreateShadersEXT failed");
            }
            shader.setShader(shaderEXT);
        }

        vector<char> readFile(const string& fileName) {
            filesystem::path filepath = shaderDirectory;
            filepath /= fileName;
            filepath += ".spv";
            ifstream file{filepath, std::ios::ate | std::ios::binary};
            if (!file.is_open()) {
                die("failed to open file : ", fileName);
            }
            const size_t fileSize = static_cast<size_t>(file.tellg());
            vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();
            return buffer;
        }

    public:
        Renderpass(const Renderpass&) = delete;
        Renderpass &operator=(const Renderpass&) = delete;
    };

}
