module;
#include <volk.h>

module z0;

import :Device;
import :DepthFrameBuffer;

namespace z0 {

    DepthFrameBuffer::DepthFrameBuffer(const Device &dev, const bool isMultisampled):
        FrameBuffer{dev},
        multisampled{isMultisampled} {
        DepthFrameBuffer::createImagesResources();
    }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    void DepthFrameBuffer::createImagesResources() {
        createImage(device.getSwapChainExtent().width,
                    device.getSwapChainExtent().height,
                    device.findImageTilingSupportedFormat(
                            {VK_FORMAT_D16_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32, VK_FORMAT_D32_SFLOAT},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                    multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    multisampled
                    ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                    : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    }


}
