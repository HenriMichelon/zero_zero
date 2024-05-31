#pragma once

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