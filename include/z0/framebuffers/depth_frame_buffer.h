#pragma once

namespace z0 {

    /**
     * Depth rendering attachment or resolved offscreen depth buffer
     */
    class DepthFrameBuffer: public BaseFrameBuffer {
    public:
        explicit DepthFrameBuffer(const Device &dev, bool multisampled);
        void createImagesResources();
    private:
        bool multisampled;
    };

}