module;
#include <volk.h>

export module z0:ColorFrameBuffer;

import :Device;
import :FrameBuffer;
import :ColorFrameBufferHDR;

export namespace z0 {

    /**
     * Default color rendering attachments
     */
    class ColorFrameBuffer: public FrameBuffer {
    public:
        // If multisampled==true attachment will support multisampling *and* HDR
        explicit ColorFrameBuffer(const Device &dev, const bool isMultisampled) :
        FrameBuffer{dev}, multisampled{isMultisampled} {
            ColorFrameBuffer::createImagesResources();
        }

        // https://vulkan-tutorial.com/Multisampling#page_Setting-up-a-render-target
        void createImagesResources() override {
            createImage(device.getSwapChainExtent().width,
                        device.getSwapChainExtent().height,
                        multisampled ? ColorFrameBufferHDR::renderFormat : device.getSwapChainImageFormat(),
                        multisampled ? device.getSamples() : VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        }

    private:
        bool multisampled;
    };

}