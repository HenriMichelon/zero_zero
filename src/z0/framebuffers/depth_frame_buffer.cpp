#include "z0/z0.h"

namespace z0 {

    DepthFrameBuffer::DepthFrameBuffer(const Device &dev, bool _multisampled):
        BaseFrameBuffer{dev},
        multisampled{_multisampled} {
         createImagesResources();
     }

    // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
    void DepthFrameBuffer::createImagesResources() {
        createImage(device.getSwapChainExtent().width,
                    device.getSwapChainExtent().height,
                    device.findImageTilingSupportedFormat(
                            // "Prefer using 24-bit depth formats for optimal performance."
                            //https://developer.nvidia.com/blog/vulkan-dos-donts/   
                            {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT),
                    multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    multisampled ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
    }

}