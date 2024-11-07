/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>

module z0.DepthFrameBuffer;

import z0.Application;
import z0.Device;

namespace z0 {

    DepthFrameBuffer::DepthFrameBuffer(const Device &dev, const bool isMultisampled):
        FrameBuffer{dev},
        multisampled{isMultisampled} {
        DepthFrameBuffer::createImagesResources();
    }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    // https://docs.vulkan.org/guide/latest/depth.html
    void DepthFrameBuffer::createImagesResources() {
        createImage(device.getSwapChainExtent().width,
                    device.getSwapChainExtent().height,
                    device.findImageSupportedFormat(
                            DEPTH_BUFFER_FORMATS[static_cast<int>(Application::get().getConfig().depthFormat)],
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                    multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    multisampled
                    ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                    : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    }


}
