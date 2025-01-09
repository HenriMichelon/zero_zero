/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>

module z0.vulkan.ColorFrameBuffer;

import z0.vulkan.ColorFrameBufferHDR;
import z0.vulkan.Device;
import z0.vulkan.FrameBuffer;

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
