module;

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
    };

}
