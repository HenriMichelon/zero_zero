#pragma once

#include "z0/framebuffers/base_frame_buffer.h"

namespace z0 {

    // Rendering attachment or resolved offscreen depth buffer
    class DepthFrameBuffer: public BaseFrameBuffer {
    public:
        explicit DepthFrameBuffer(const Device &dev, bool multisampled);
        void createImagesResources();
    private:
        bool multisampled;
    };

}