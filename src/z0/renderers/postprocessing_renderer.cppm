module;
#include "z0/libraries.h"
#include <volk.h>

export module Z0:PostprocessingRenderer;

import :Tools;
import :Renderer;
import :Renderpass;
import :Device;
import :SampledFrameBuffer;
import :ColorFrameBufferHDR;
import :Descriptors;

export namespace z0 {

    /**
     * Base class for post processing effect renderers
     */
    class PostprocessingRenderer: public Renderpass, public Renderer {
    public:
        PostprocessingRenderer(Device& device,
                                   string shaderDirectory,
                                   SampledFrameBuffer* inputColorAttachmentHdr);

        void setInputColorAttachmentHdr(SampledFrameBuffer* inputColorAttachmentHdr);
        [[nodiscard]] shared_ptr<ColorFrameBufferHDR>& getColorAttachment() { return colorAttachmentHdr; }
        [[nodiscard]] VkImage getImage() const override { return colorAttachmentHdr->getImage(); }
        [[nodiscard]] VkImageView getImageView() const override { return colorAttachmentHdr->getImageView(); }

        void cleanup() override;
        void loadShaders() override;
        void createOrUpdateDescriptorSet(bool create) override;
        void createDescriptorSetLayout() override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;
        void createImagesResources() override;
        void cleanupImagesResources() override;
        void recreateImagesResources() override;
        void beginRendering(VkCommandBuffer commandBuffer) override;
        void endRendering(VkCommandBuffer commandBuffer, bool isLast) override;

    protected:
        shared_ptr<ColorFrameBufferHDR> colorAttachmentHdr;
        SampledFrameBuffer* inputColorAttachmentHdr;
    };

     PostprocessingRenderer::PostprocessingRenderer(Device &dev,
                                                           string shaderDirectory,
                                                           SampledFrameBuffer* inputColorAttachmentHdr):
            Renderpass{dev, shaderDirectory}, inputColorAttachmentHdr{inputColorAttachmentHdr} {
        createImagesResources();
    }

    void PostprocessingRenderer::cleanup() {
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    void PostprocessingRenderer::loadShaders() {
        vertShader = createShader("quad.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void PostprocessingRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        bindShaders(commandBuffer);
        vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        setViewport(commandBuffer, device.getSwapChainExtent().width, device.getSwapChainExtent().height);
        vkCmdSetVertexInputEXT(commandBuffer, 0, nullptr, 0, nullptr);
        vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
        bindDescriptorSets(commandBuffer, currentFrame);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

    void PostprocessingRenderer::createImagesResources() {
        colorAttachmentHdr = make_shared<ColorFrameBufferHDR>(device);
    }

    void PostprocessingRenderer::cleanupImagesResources() {
        colorAttachmentHdr->cleanupImagesResources();
    }

    void PostprocessingRenderer::recreateImagesResources() {
        colorAttachmentHdr->cleanupImagesResources();
        colorAttachmentHdr->createImagesResources();
    }

    void PostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool =DescriptorPool::Builder(device)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
                .build();
        setLayout = DescriptorSetLayout::Builder(device)
                .addBinding(0,
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(1,
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT,
                            1)
            .build();
        createUniformBuffers(globalUniformBuffers, globalUniformBufferSize);
    }

    void PostprocessingRenderer::setInputColorAttachmentHdr(SampledFrameBuffer* input) {
        inputColorAttachmentHdr = input;
        createOrUpdateDescriptorSet(false);
    }

    void PostprocessingRenderer::createOrUpdateDescriptorSet(bool create) {
        for (uint32_t i = 0; i < descriptorSet.size(); i++) {
            auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto imageInfo = inputColorAttachmentHdr->imageInfo();
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                    .writeBuffer(0, &globalBufferInfo)
                    .writeImage(1, &imageInfo);
            if (create) {
                if (!writer.build(descriptorSet[i])) {
                    die("Cannot allocate descriptor set for BasePostprocessingRenderer");
                }
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void PostprocessingRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        device.transitionImageLayout(commandBuffer, colorAttachmentHdr->getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorAttachmentHdr->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, device.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = nullptr,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void PostprocessingRenderer::endRendering(VkCommandBuffer commandBuffer, bool isLast) {
        vkCmdEndRendering(commandBuffer);
        device.transitionImageLayout(commandBuffer, colorAttachmentHdr->getImage(),
                                        VK_IMAGE_LAYOUT_UNDEFINED,
                                        isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                        0,
                                        isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                        VK_IMAGE_ASPECT_COLOR_BIT);
    }

}