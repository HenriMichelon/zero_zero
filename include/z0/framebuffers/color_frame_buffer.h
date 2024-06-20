#pragma once

namespace z0 {

    /**
     * Default color rendering attachments
     */
    class ColorFrameBuffer: public BaseFrameBuffer {
    public:
        // If multisampled==true attachment will support multisampling *and* HDR
        explicit ColorFrameBuffer(const Device &dev, bool multisampled);
        void createImagesResources() override;
    private:
        bool multisampled;
    };

}