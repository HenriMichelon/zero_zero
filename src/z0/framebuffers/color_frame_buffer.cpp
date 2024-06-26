#include "z0/z0.h"

namespace z0 {

    ColorFrameBuffer::ColorFrameBuffer(const Device &dev, bool _multisampled) :
        FrameBuffer{dev}, multisampled{_multisampled} {
         createImagesResources();
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