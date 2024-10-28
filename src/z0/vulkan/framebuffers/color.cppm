module;

export module z0:ColorFrameBuffer;

import :Device;
import :FrameBuffer;

export namespace z0 {

    /*
     * Default color rendering attachments
     */
    class ColorFrameBuffer: public FrameBuffer {
    public:
        // If multisampled==true attachment will support multisampling *and* HDR
        explicit ColorFrameBuffer(const Device &dev, bool isMultisampled);

        void createImagesResources() override ;

    private:
        bool multisampled;
    };

}