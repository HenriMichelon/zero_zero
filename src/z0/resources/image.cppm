module;
#include "z0/libraries.h"

export module z0:Image;

import :Constants;
import :Resource;

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
         * Load a bitmap from file. For supported file formats see https://github.com/nothings/stb/blob/master/stb_image.h
         */
        static shared_ptr<Image> load(const string &filepath, ImageFormat imageFormat);

        static shared_ptr<Image> createBlankImage();

        static shared_ptr<Image> create(uint32_t width, uint32_t height, uint64_t imageSize, const void *data, const string & name);

    protected:
        uint32_t width;
        uint32_t height;

        Image(uint32_t width, uint32_t height, const string & name);
    };

}
