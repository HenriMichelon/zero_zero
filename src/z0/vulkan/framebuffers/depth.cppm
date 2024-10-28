module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:DepthFrameBuffer;

import :Device;
import :FrameBuffer;

export namespace z0 {

    /**
     * Depth rendering attachment or resolved offscreen depth buffer
     */
    class DepthFrameBuffer : public FrameBuffer {
    public:
        explicit DepthFrameBuffer(const Device &dev, bool isMultisampled);

        void createImagesResources() override;

    private:
        bool multisampled;

        const vector<VkFormat> DEPTH_BUFFER_FORMATS[4] = {
            // DEPTH_FORMAT_AUTO
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DEPTH_FORMAT_16BIT
            {
                VK_FORMAT_D16_UNORM,
                VK_FORMAT_D16_UNORM_S8_UINT,
            },
            // DEPTH_FORMAT_24BIT
            {
                VK_FORMAT_X8_D24_UNORM_PACK32,
                VK_FORMAT_D24_UNORM_S8_UINT,
            },
            // DEPTH_FORMAT_32BIT
            {
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
            },

        };
    };

}
