module;
#include <cassert>
#include "z0/libraries.h"

module z0;

import :Constants;
import :Color;
import :Texture;
import :Material;

namespace z0 {

    Material::Material(const string &name):
        Resource(name) {
    }

    StandardMaterial::StandardMaterial(const string &name):
        Material(name) {
    }

    ShaderMaterial::ShaderMaterial(const shared_ptr<ShaderMaterial> &orig):
        Material{orig->name},
        fragFileName{orig->fragFileName},
        vertFileName{orig->vertFileName} {
        for (int i = 0; i < MAX_PARAMETERS; i++) {
            parameters[i] = orig->parameters[i];
        }
    }

    ShaderMaterial::ShaderMaterial(string        fragShaderFileName,
                                   string        vertShaderFileName,
                                   const string &name):
        Material{name},
        fragFileName{std::move(fragShaderFileName)},
        vertFileName{std::move(vertShaderFileName)} {
    }

    shared_ptr<ShaderMaterial> &OutlineMaterials::get(const int index) {
        assert(index < materials.size());
        return materials[index];
    }

    void OutlineMaterials::add(const shared_ptr<ShaderMaterial> &material) {
        materials.push_back(material);
    }

    void OutlineMaterials::_initialize() {
        const auto outlineMaterial = make_shared<ShaderMaterial>("outline.frag", "outline.vert");
        outlineMaterial->setParameter(0, {0.0f, 0.0f, 0.0f, 1.0f});
        outlineMaterial->setParameter(1, vec4{0.01f});
        add(outlineMaterial);
    }

    vector<shared_ptr<ShaderMaterial>> OutlineMaterials::materials;

}
