module;
#include <volk.h>

module z0;

import :Device;
import :FrameBuffer;
import :ColorFrameBuffer;
import :ColorFrameBufferHDR;

namespace z0 {

    ColorFrameBuffer::ColorFrameBuffer(const Device &dev, const bool isMultisampled) :
        FrameBuffer{dev}, multisampled{isMultisampled} {
        ColorFrameBuffer::createImagesResources();
    }

    // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
    void ColorFrameBuffer::createImagesResources() {
        createImage(device.getSwapChainExtent().width,
                    device.getSwapChainExtent().height,
                    multisampled ? ColorFrameBufferHDR::renderFormat : device.getSwapChainImageFormat(),
                    multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    }


}
