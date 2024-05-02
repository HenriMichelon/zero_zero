#pragma once

#include <utility>

#include "z0/color.h"
#include "z0/resources/texture.h"

namespace z0 {

    class Material: public Resource {
    public:
        CullMode getCullMode() const { return cullMode;}
        void setCullMode(CullMode mode) {cullMode = mode;}

    protected:
        explicit Material(const string& name): Resource(name) {}

    private:
        CullMode cullMode { CULLMODE_BACK };
    };

    class StandardMaterial: public Material {
    public:
        explicit StandardMaterial(const string& name = "StandardMaterial"): Material(name) {}

        const Color &getAlbedoColor() const { return albedoColor;}
        void setAlbedoColor(const Color &color) { albedoColor = color;}

        const shared_ptr<ImageTexture> &getAlbedoTexture() const { return albedoTexture;}
        void setAlbedoTexture(const shared_ptr<ImageTexture> &texture) { albedoTexture = texture; }

        const shared_ptr<ImageTexture> &getSpecularTexture() const { return specularTexture;}
        void setSpecularTexture(const shared_ptr<ImageTexture> &texture) {specularTexture = texture;}

        const shared_ptr<ImageTexture> &getNormalTexture() const {return normalTexture;}
        void setNormalTexture(const shared_ptr<ImageTexture> &texture) {normalTexture = texture;}

        Transparency getTransparency() const { return transparency;}
        void setTransparency(Transparency transparencyMode) {transparency = transparencyMode;}

        float getAlphaScissor() const { return alphaScissor;}
        void setAlphaScissor(float scissor) {alphaScissor = scissor;}

    private:
        Color                      albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        shared_ptr<ImageTexture>   albedoTexture {nullptr};
        shared_ptr<ImageTexture>   specularTexture {nullptr};
        shared_ptr<ImageTexture>   normalTexture {nullptr};
        Transparency               transparency { TRANSPARENCY_DISABLED };
        float                      alphaScissor { 0.1 };
    };

    class ShaderMaterial: public Material {
    public:
        explicit ShaderMaterial(string shaderFileName, const string& name = "ShaderMaterial"): Material(name), fileName{std::move(shaderFileName)} {}

        const string& getFileName() const { return fileName; }
    private:
        const string fileName;
    };

}