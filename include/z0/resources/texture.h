#pragma once

namespace z0 {

    /**
     * Base class the textures resource.
     */
    class Texture: public Resource {
    public:
        /**
         * Creates a Texture
         * @param name resource name. Only informative.
         */
        explicit Texture(const string& name): Resource{name} {};

        /**
         * Returns the width in pixels on the texture
         */
        virtual uint32_t getWidth() const = 0;

        /**
         * Returns the height in pixels on the texture
         */
        virtual uint32_t getHeight() const = 0;

        /**
         * Returns the size in pixels on the texture
         */
        [[nodiscard]] virtual vec2 getSize() const { return vec2{getWidth(), getHeight()}; };
    };

    /**
     * Image based texture stored in GPU memory
     */
    class ImageTexture: public Texture {
    public:
        /**
         * Creates an ImageTexture from an existing Image
         */
        explicit ImageTexture(const shared_ptr<Image>& img): Texture{img->getName()}, image(img) {};

        /**
         * Creates an ImageTexture from a image file
         * @param filename : image file name, relative to the application working directory
         */
        explicit ImageTexture(const string& filename);

        /**
         * Returns the attached Image
         */
        [[nodiscard]] const shared_ptr<Image>& getImage() const { return image; }

        [[nodiscard]] uint32_t getWidth() const override { return image->getWidth(); };
        [[nodiscard]] uint32_t getHeight() const override { return image->getHeight(); };

    protected:
        shared_ptr<Image> image {nullptr};
    };


}