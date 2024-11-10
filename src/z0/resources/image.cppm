/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Image;

import z0.Constants;
import z0.Resource;

export namespace z0 {

    /**
     * A bitmap resource, stored in GPU memory.
     */
    class Image : public Resource {
    public:
        ~Image() override = default;

        /**
         * Returns the width in pixels
         */
        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels
         */
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        /**
         * Returns the size in pixels
         */
        [[nodiscard]] inline vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        /**
         * Load a bitmap from file.<br>
         * Support JPEG, PNG, DDS and KTX2 formats.
         */
        static shared_ptr<Image> load(const string &filepath, ImageFormat imageFormat = ImageFormat::R8G8B8A8_SRGB);

        /**
         * Load a bitmap from memory.<br>
         * Support JPEG & PNG formats.
         */
        static shared_ptr<Image> load(const void* data, uint64_t dataSize, ImageFormat imageFormat = ImageFormat::R8G8B8A8_SRGB);

        static shared_ptr<Image> createBlankImage();

        /**
         * Creates an image from memory
         * @param width width in pixels
         * @param height height in pixels
         * @param imageSize size in bytes
         * @param data memory bloc address
         * @param name name of the resource
         * @param format image pixel format
         */
        static shared_ptr<Image> create(uint32_t width, uint32_t height,
                                        uint64_t imageSize, const void *data,
                                        const string & name, ImageFormat format = ImageFormat::R8G8B8A8_SRGB);

    protected:
        uint32_t width;
        uint32_t height;

        Image(uint32_t width, uint32_t height, const string & name);
    };

}
