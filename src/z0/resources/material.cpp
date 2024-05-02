#include "z0/resources/material.h"

namespace z0 {

    void StandardMaterial::setAlbedoColor(const Color &color) {
        albedoColor = color;
        updated = true;
    }

    void StandardMaterial::setAlbedoTexture(const shared_ptr<ImageTexture> &texture) {
        albedoTexture = texture;
        updated = true;
    }

    void StandardMaterial::setSpecularTexture(const shared_ptr<ImageTexture> &texture) {
        specularTexture = texture;
        updated = true;
    }

    void StandardMaterial::setNormalTexture(const shared_ptr<ImageTexture> &texture) {
        normalTexture = texture;
        updated = true;
    }

    void StandardMaterial::setCullMode(CullMode mode) {
        cullMode = mode;
        updated = true;
    }

    void StandardMaterial::setTransparency(Transparency transparencyMode) {
        transparency = transparencyMode;
        updated = true;
    }

    void StandardMaterial::setAlphaScissor(float scissor) {
        alphaScissor = scissor;
        updated = true;
    }

}
