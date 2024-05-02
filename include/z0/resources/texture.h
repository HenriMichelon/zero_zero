#pragma once

#include <utility>

#include "z0/resources/image.h"

namespace z0 {

    class Texture: public Resource {
    public:
        explicit Texture(const string& resName): Resource{resName} {};
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;
        virtual vec2 getSize() const { return vec2{getWidth(), getHeight()}; };
    };

    class ImageTexture: public Texture {
    public:
        explicit ImageTexture(const shared_ptr<Image>& img): Texture{img->getName()}, image(img) {};
        explicit ImageTexture(const string& filename);

        Image& getImage() { return *image; }
        uint32_t getWidth() const override { return image->getWidth(); };
        uint32_t getHeight() const override { return image->getHeight(); };

    protected:
        shared_ptr<Image> image {nullptr};

    public:
        shared_ptr<Image> _getImagePointer() { return image; }
    };


}