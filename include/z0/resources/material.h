#pragma once

#include <utility>

#include "z0/color.h"
#include "z0/resources/texture.h"

namespace z0 {

    class Material: public Resource {
    public:
        explicit Material(const string& name): Resource(name) {}

    protected:
        bool updated{false};

    public:
        bool _isUpdated() const { return updated; }
        void _resetUpdate() { updated = false; }
    };

    class StandardMaterial: public Material {
    public:
        explicit StandardMaterial(const string& name): Material(name) {}

        const Color &getAlbedoColor() const { return albedoColor;}
        void setAlbedoColor(const Color &color);

        const shared_ptr<ImageTexture> &getAlbedoTexture() const { return albedoTexture;}
        void setAlbedoTexture(const shared_ptr<ImageTexture> &texture);

        const shared_ptr<ImageTexture> &getSpecularTexture() const { return specularTexture;}
        void setSpecularTexture(const shared_ptr<ImageTexture> &texture);

        const shared_ptr<ImageTexture> &getNormalTexture() const {return normalTexture;}
        void setNormalTexture(const shared_ptr<ImageTexture> &texture);

        CullMode getCullMode() const { return cullMode;}
        void setCullMode(CullMode mode);

        Transparency getTransparency() const { return transparency;}
        void setTransparency(Transparency transparencyMode);

        float getAlphaScissor() const { return alphaScissor;}
        void setAlphaScissor(float scissor);

    private:
        Color                      albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        shared_ptr<ImageTexture>   albedoTexture {nullptr};
        shared_ptr<ImageTexture>   specularTexture {nullptr};
        shared_ptr<ImageTexture>   normalTexture {nullptr};
        CullMode                   cullMode { CULLMODE_BACK };
        Transparency               transparency { TRANSPARENCY_DISABLED };
        float                      alphaScissor { 0.1 };
    };

}