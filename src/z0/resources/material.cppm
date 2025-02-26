/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"

export module z0.resources.Material;

import z0.Constants;

import z0.resources.Resource;
import z0.resources.Texture;

export namespace z0 {
    /**
     * Base class for all materials of models surfaces
     */
    class Material : public Resource {
    public:
        /**
         * Returns the cull mode.
         */
        [[nodiscard]] inline auto getCullMode() const { return cullMode; }

        /**
         * Sets the CullMode.
         * Determines which side of the triangle to cull depending on whether the triangle faces towards or away from the camera.
         */
        inline void setCullMode(const CullMode mode) { cullMode = mode; }

        /**
         * Returns the transparency mode
         */
        [[nodiscard]] inline auto getTransparency() const { return transparency; }

        /**
         * Sets the transparency mode
         */
        inline void setTransparency(const Transparency transparencyMode) { transparency = transparencyMode; }

        /**
         * Returns the alpha scissor threshold value
         */
        [[nodiscard]] inline auto getAlphaScissor() const { return alphaScissor; }

        /**
         * Sets the alpha scissor threshold value
         * Threshold at which the alpha scissor will discard values.
         * Higher values will result in more pixels being discarded.
         * If the material becomes too opaque at a distance, try increasing this value.
         * If the material disappears at a distance, try decreasing this value.
         */
        inline void setAlphaScissor(const float scissor) { alphaScissor = scissor; }

    protected:
        explicit Material(const string &name);

    private:
        CullMode     cullMode{CullMode::DISABLED};
        Transparency transparency{Transparency::DISABLED};
        float        alphaScissor{0.1f};
        // The material parameters will be written in GPU memory next frame
        uint32_t     dirty;

    public:
        inline auto _isDirty() const { return dirty > 0; }
        void _setDirty();
        inline auto _clearDirty() { this->dirty--; }
    };

    /**
     * Simple albedo/specular/normal material
     */
    class StandardMaterial : public Material {
    public:
        /**
         * References and properties of a texture
         */
        struct TextureInfo {
            shared_ptr<ImageTexture> texture{nullptr};
            mat3                     transform{1.0f};
        };

        /**
         * Creates a StandardMaterial with default parameters
         */
        explicit StandardMaterial(const string &name = "StandardMaterial");

        /**
         * Returns the material's base color.
         */
        [[nodiscard]] inline const auto& getAlbedoColor() const { return albedoColor; }

        /**
         * Sets the material's base color.
         */
        inline void setAlbedoColor(const vec4 &color) { albedoColor = color; }

        /**
         * Returns the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        [[nodiscard]] inline const auto& getAlbedoTexture() const { return albedoTexture; }

        /**
         * Sets the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        void setAlbedoTexture(const TextureInfo &texture);

        /**
         * Return the normal texture
         */
        [[nodiscard]] inline const auto &getNormalTexture() const { return normalTexture; }

        /**
         * Sets the normal texture
         */
        void setNormalTexture(const TextureInfo &texture);

        [[nodiscard]] inline auto getMetallicFactor() const { return metallicFactor; }

        void setMetallicFactor(float metallic);

        [[nodiscard]] inline const auto& getMetallicTexture() const { return metallicTexture; }

        void setMetallicTexture(const TextureInfo &texture);

        [[nodiscard]] inline float getRoughnessFactor() const { return roughnessFactor; }

        void setRoughnessFactor(float roughness);

        [[nodiscard]] inline const auto& getRoughnessTexture() const { return roughnessTexture; }

        void setRoughnessTexture(const TextureInfo &texture);

        [[nodiscard]] inline const auto& getEmissiveTexture() const { return emissiveTexture; }

        [[nodiscard]] inline vec3 getEmissiveFactor() const { return emissiveFactor; }

        void setEmissiveFactor(const vec3& emissive);

        void setEmissiveTexture(const TextureInfo& texture);

        [[nodiscard]] inline auto getEmissiveStrength() const { return emissiveStrength; }

        void setEmissiveStrength(float emissive);

        [[nodiscard]] inline auto getNormalScale() const { return normalScale; }

        void setNormalScale(float scale);

    private:
        vec4         albedoColor{1.0f, 0.0f, 0.5f, 1.0f};
        TextureInfo  albedoTexture{};
        float        metallicFactor{-1.0f}; // -1 -> non PBR material
        TextureInfo  metallicTexture{};
        float        roughnessFactor{1.0f};
        TextureInfo  roughnessTexture{};
        vec3         emissiveFactor{0.0f};
        float        emissiveStrength{1.0f};
        TextureInfo  emissiveTexture;
        TextureInfo  normalTexture{};
        float        normalScale{1.0f};
    };

    /**
     * Shader based material
     */
    class ShaderMaterial : public Material {
    public:
        /**
         * Maximum number of parameters of a ShaderMaterial
         */
        static constexpr int MAX_PARAMETERS = 4;

        /**
         * Creates a ShaderMaterial by copy
         */
        explicit ShaderMaterial(const shared_ptr<ShaderMaterial> &orig);

        /**
         * Creates a ShaderMaterial
         * @param fragShaderFileName fragment shader file path, relative to the application directory
         * @param vertShaderFileName vertex shader file path, relative to the application directory
         * @param name Resource name
         */
        explicit ShaderMaterial(const string &fragShaderFileName, const string &vertShaderFileName = "",
                                const string &name               = "ShaderMaterial");

        /**
         * Returns the fragment shader file path, relative to the application directory
         */
        [[nodiscard]] inline const auto& getFragFileName() const { return fragFileName; }

        /**
         * Returns the vertex shader file path, relative to the application directory
         */
        [[nodiscard]] inline const auto& getVertFileName() const { return vertFileName; }

        /**
         * Sets a parameter value
         */
        void setParameter(int index, vec4 value);

        /**
         * Returns a parameter value
         */
        [[nodiscard]] inline auto getParameter(const int index) const { return parameters[index]; }

    private:
        const string fragFileName;
        const string vertFileName;
        vec4         parameters[MAX_PARAMETERS]{};
    };

    /**
     * Singleton for all the materials used by the SceneRenderer to outline meshes
     */
    class OutlineMaterials {
    public:
        OutlineMaterials();

        /**
         * Returns a given outline material
         */
        const shared_ptr<ShaderMaterial> &get(int index) const;

        /**
         * Adds an outline material.
         */
        void add(const shared_ptr<ShaderMaterial> &material);

        inline const auto &getAll() const {
            return materials;
        }

    private:
        vector<shared_ptr<ShaderMaterial>> materials;
    };

}
