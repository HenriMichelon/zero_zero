/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"

module z0.resources.Material;

import z0.Application;
import z0.Constants;

import z0.resources.Texture;

namespace z0 {

    Material::Material(const string &name):
        Resource(name),
        dirty{app().getConfig().framesInFlight}{
    }

    void Material::_setDirty() {
        dirty = app().getConfig().framesInFlight;
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
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
        _setDirty();
    }

    void StandardMaterial::setRoughnessTexture(const TextureInfo &texture) {
        roughnessTexture = texture;
        if (metallicFactor == -1.0f) { metallicFactor = 0.0f; }
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

    void StandardMaterial::setNormalScale(const float scale) {
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

    ShaderMaterial::ShaderMaterial(const string &fragShaderFileName, const string &vertShaderFileName,
                                   const string &name):
        Material{name},
        fragFileName{fragShaderFileName},
        vertFileName{vertShaderFileName} {
    }

    void ShaderMaterial::setParameter(const int index, const vec4 value) {
        parameters[index] = value;
        _setDirty();
    }

    const shared_ptr<ShaderMaterial> &OutlineMaterials::get(const int index) const {
        assert(index < materials.size());
        return materials[index];
    }

    void OutlineMaterials::add(const shared_ptr<ShaderMaterial> &material) {
        materials.push_back(material);
    }

    OutlineMaterials::OutlineMaterials() {
        const auto outlineMaterial = make_shared<ShaderMaterial>("outline.frag", "outline.vert");
        outlineMaterial->setParameter(0, {0.58f, 0.478f, 0.086f, 1.0f});
        outlineMaterial->setParameter(1, vec4{0.01f});
        add(outlineMaterial);
    }

}
