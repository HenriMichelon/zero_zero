#pragma once

#include <utility>

#include "z0/color.h"
#include "z0/resources/texture.h"

namespace z0 {

    // TODO : manage dirty materials to update the uniform buffers
    class Material: public Resource {
    public:
        explicit Material(const string& name): Resource(name) {}
    };

    class StandardMaterial: public Material {
    public:
        Color                      albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        shared_ptr<ImageTexture>   albedoTexture {nullptr};
        shared_ptr<ImageTexture>   specularTexture {nullptr};
        shared_ptr<ImageTexture>   normalTexture {nullptr};
        CullMode                   cullMode { CULLMODE_BACK };
        Transparency               transparency { TRANSPARENCY_DISABLED };
        float                      alphaScissor { 0.1 };

        explicit StandardMaterial(const string& name): Material(name) {}
    };

}