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

    void StandardMaterial::setAlbedoTexture(const shared_ptr<ImageTexture> &texture) {
        albedoTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setSpecularTexture(const shared_ptr<ImageTexture> &texture) {
        specularTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setNormalTexture(const shared_ptr<ImageTexture> &texture) {
        normalTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setMetallicTexture(const shared_ptr<ImageTexture> &texture) {
        metallicTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setRoughnessTexture(const shared_ptr<ImageTexture> &texture) {
        roughnessTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setOcclusionTexture(const shared_ptr<ImageTexture> &texture) {
        occlusionTexture = texture;
        _setDirty();
    }

    void StandardMaterial::setTextureTransform(const TextureTransform transform) {
        textureTransform = make_shared<TextureTransform>(transform);
        _setDirty();
    }

    // void StandardMaterial::setMetallicTextureChannel(const TextureChannel channel) {
    //     metallicTextureChannel = channel;
    //     _setDirty();
    // }
    //
    // void StandardMaterial::setRoughnessTextureChannel(const TextureChannel channel) {
    //     roughnessTextureChannel = channel;
    //     _setDirty();
    // }

    void StandardMaterial::setMetallic(const float metallic) {
        this->metallic = metallic;
        _setDirty();
    }

    void StandardMaterial::setRoughness(const float roughness) {
        this->roughness = roughness;
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
