/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.vulkan.ColorFrameBuffer;

import z0.vulkan.Device;
import z0.vulkan.FrameBuffer;

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