module;
#include "volk.h"
#include "z0/libraries.h"

module z0;

import :PostprocessingRenderer;
import :Device;
import :ColorFrameBufferHDR;
import :TonemappingPostprocessingRenderer;
import :DepthFrameBuffer;
import :Descriptors;

namespace z0 {

    TonemappingPostprocessingRenderer::TonemappingPostprocessingRenderer(
            Device &            device,
            const string &      shaderDirectory,
            const vector<shared_ptr<ColorFrameBufferHDR>>&inputColorAttachmentHdr,
            const vector<shared_ptr<DepthFrameBuffer>>& depthBuffer):
        PostprocessingRenderer{device, shaderDirectory, inputColorAttachmentHdr},
        resolvedDepthBuffer{depthBuffer} {
        createOrUpdateResources();
    }

    void TonemappingPostprocessingRenderer::update(const uint32_t currentFrame) {
        const GlobalUniformBufferObject globalUbo {
            .gamma = Application::get().getConfig().gamma,
            .exposure = Application::get().getConfig().exposure,
        };
        writeUniformBuffer(globalUniformBuffers[currentFrame], &globalUbo);
    }

    void TonemappingPostprocessingRenderer::createDescriptorSetLayout() {
        descriptorPool = DescriptorPool::Builder(device)
                                 .setMaxSets(device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.getFramesInFlight())
                                 .build();
        setLayout = DescriptorSetLayout::Builder(device)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            // source color attachment
                            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            // source depth attachment
                            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
                            .build();
        globalUniformBufferSize = sizeof(GlobalUniformBufferObject);
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            globalUniformBuffers[i] = createUniformBuffer(globalUniformBufferSize);
        }
    }

    void TonemappingPostprocessingRenderer::createOrUpdateDescriptorSet(const bool create) {
        for (auto i = 0; i < device.getFramesInFlight(); i++) {
            auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto colorInfo        = inputColorAttachmentHdr[i]->imageInfo();
            auto depthInfo        = VkDescriptorImageInfo {
                .sampler = colorInfo.sampler,
                .imageView = resolvedDepthBuffer[i]->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
            };
            auto writer           = DescriptorWriter(*setLayout, *descriptorPool)
                                        .writeBuffer(0, &globalBufferInfo)
                                        .writeImage(1, &colorInfo)
                                        .writeImage(2, &depthInfo);
            if (create) {
                if (!writer.build(descriptorSet[i])) {
                    die("Cannot allocate descriptor set for BasePostprocessingRenderer");
                }
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void TonemappingPostprocessingRenderer::loadShaders() {
        PostprocessingRenderer::loadShaders();
        fragShader = createShader("reinhard.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

}
