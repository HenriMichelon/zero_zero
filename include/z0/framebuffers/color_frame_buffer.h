#pragma once

#include "z0/framebuffers/base_frame_buffer.h"

namespace z0 {

    // Default Color Rendering attachments
    class ColorFrameBuffer: public BaseFrameBuffer {
    public:
        // If multisampled==true attachment will support multisampling *and* HDR
        explicit ColorFrameBuffer(const Device &dev, bool multisampled);
        void createImagesResources() override;
    private:
        bool multisampled;
    };

}