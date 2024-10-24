module;
#include <volk.h>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Tools;
import :Renderer;
import :Renderpass;
import :Device;
import :Image;
import :Descriptors;
import :Equirect2CubemapRenderer;

namespace z0 {

    Equirect2CubemapRenderer::Equirect2CubemapRenderer(Device &device,
                                                       const shared_ptr<Image>& hdrFile,
                                                       const string &shaderDirectory) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR}, hdrFile{hdrFile} {
    }

    void Equirect2CubemapRenderer::cleanup() {
        cleanupImagesResources();
        Renderpass::cleanup();
    }

    void Equirect2CubemapRenderer::loadShaders() {
        vertShader = createShader(".vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void Equirect2CubemapRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        bindShaders(commandBuffer);
    }

    void Equirect2CubemapRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
    }

    void Equirect2CubemapRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            // auto imageInfo        = inputColorAttachmentHdr[i]->imageInfo();
            // auto writer           = DescriptorWriter(*setLayout, *descriptorPool)
            //                           .writeImage(0, &imageInfo);
            // if (create) {
            //     if (!writer.build(descriptorSet[i])) {
            //         die("Cannot allocate descriptor set for BasePostprocessingRenderer");
            //     }
            // } else {
            //     writer.overwrite(descriptorSet[i]);
            // }
        }
    }

    void Equirect2CubemapRenderer::beginRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        // device.transitionImageLayout(commandBuffer,
        //                              colorAttachmentHdr[currentFrame]->getImage(),
        //                              VK_IMAGE_LAYOUT_UNDEFINED,
        //                              VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        //                              0,
        //                              VK_ACCESS_TRANSFER_WRITE_BIT,
        //                              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        //                              VK_PIPELINE_STAGE_TRANSFER_BIT,
        //                              VK_IMAGE_ASPECT_COLOR_BIT);
        // const VkRenderingAttachmentInfo colorAttachmentInfo{.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
        //                                                     .imageView   = colorAttachmentHdr[currentFrame]->getImageView(),
        //                                                     .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        //                                                     .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        //                                                     .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        //                                                     .clearValue  = clearColor};
        // const VkRenderingInfo           renderingInfo{.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        //                                               .pNext                = nullptr,
        //                                               .renderArea           = {{0, 0}, device.getSwapChainExtent()},
        //                                               .layerCount           = 1,
        //                                               .colorAttachmentCount = 1,
        //                                               .pColorAttachments    = &colorAttachmentInfo,
        //                                               .pDepthAttachment     = nullptr,
        //                                               .pStencilAttachment   = nullptr};
        // vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void Equirect2CubemapRenderer::endRendering(const VkCommandBuffer commandBuffer, const uint32_t currentFrame, const bool isLast) {
        vkCmdEndRendering(commandBuffer);
        // device.transitionImageLayout(commandBuffer,
        //                              colorAttachmentHdr[currentFrame]->getImage(),
        //                              VK_IMAGE_LAYOUT_UNDEFINED,
        //                              isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
        //                                     : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        //                              0,
        //                              isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
        //                              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //                              isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        //                              VK_IMAGE_ASPECT_COLOR_BIT);
    }

} // namespace z0
