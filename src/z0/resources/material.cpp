/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
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
        Resource(name),
        dirty{Application::get().getConfig().framesInFlight}{
    }

    void Material::_setDirty() {
        dirty = Application::get().getConfig().framesInFlight;
    }

    StandardMaterial::StandardMaterial(const string &name):
        Material(name) {
    }

    void StandardMaterial::setAlbedoTexture(const TextureInfo &texture) {
        albedoTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setNormalTexture(const TextureInfo &texture) {
        normalTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setMetallicTexture(const TextureInfo &texture) {
        metallicTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setRoughnessTexture(const TextureInfo &texture) {
        roughnessTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setEmissiveTexture(const TextureInfo &texture) {
        emissiveTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setMetallicFactor(const float metallic) {
        this->metallicFactor = metallic;
        _setDirty();
    }

    void StandardMaterial::setRoughnessFactor(const float roughness) {
        this->roughnessFactor = roughness;
        _setDirty();
    }

    void StandardMaterial::setEmissiveFactor(const vec3& factor) {
        emissiveFactor = factor;
        _setDirty();
    }

    void StandardMaterial::setEmissiveStrength(const float strength) {
        emissiveStrength = strength;
        _setDirty();
    }

    void StandardMaterial::setNormaleScale(const float scale) {
        normalScale = scale;
        _setDirty();
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

    void ShaderMaterial::setParameter(const int index, const vec4 value) {
        parameters[index] = value;
        _setDirty();
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
