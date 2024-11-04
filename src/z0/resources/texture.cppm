/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0:Texture;

import :Constants;
import :Resource;
import :Image;

export namespace z0 {

    /**
     * Base class for textures resources.
     */
    class Texture : public Resource {
    public:
        /**
         * Creates a Texture
         * @param name resource name. Only informative.
         */
        explicit Texture(const string &name);

        /**
         * Returns the width in pixels on the texture
         */
        [[nodiscard]] virtual uint32_t getWidth() const = 0;

        /**
         * Returns the height in pixels on the texture
         */
        [[nodiscard]] virtual uint32_t getHeight() const = 0;

        /**
         * Returns the size in pixels on the texture
         */
        [[nodiscard]] virtual vec2 getSize() const { return vec2{getWidth(), getHeight()}; }
    };

    /**
     * Image based texture stored in GPU memory
     */
    class ImageTexture : public Texture {
    public:
        /**
         * Creates an ImageTexture from an existing Image
         */
        explicit ImageTexture(const shared_ptr<Image> &img);

        /**
         * Creates an ImageTexture from an image resource
         */
        explicit ImageTexture(const string &filename, ImageFormat imageFormat = IMAGE_R8G8B8A8_SRGB);

        /**
         * Returns the attached Image
         */
        [[nodiscard]] inline const shared_ptr<Image> &getImage() const { return image; }

        [[nodiscard]] inline uint32_t getWidth() const override { return image->getWidth(); }

        [[nodiscard]] inline uint32_t getHeight() const override { return image->getHeight(); }

    protected:
        shared_ptr<Image> image{nullptr};
    };

}
