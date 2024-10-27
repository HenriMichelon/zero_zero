module;
#include <cassert>
#include "z0/libraries.h"

export module z0:Material;

import :Constants;
import :Resource;
import :Color;
import :Texture;

export namespace z0 {
    /**
     * Base class for all materials of models surfaces
     */
    class Material : public Resource {
    public:
        /**
         * Returns the cull mode.
         */
        [[nodiscard]] inline CullMode getCullMode() const { return cullMode; }

        /**
         * Sets the CullMode.
         * Determines which side of the triangle to cull depending on whether the triangle faces towards or away from the camera.
         */
        inline void setCullMode(const CullMode mode) { cullMode = mode; }

        /**
         * Returns the transparency mode
         */
        [[nodiscard]] inline Transparency getTransparency() const { return transparency; }

        /**
         * Sets the transparency mode
         */
        inline void setTransparency(const Transparency transparencyMode) { transparency = transparencyMode; }

        /**
         * Returns the alpha scissor threshold value
         */
        [[nodiscard]] inline float getAlphaScissor() const { return alphaScissor; }

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
        CullMode     cullMode{CULLMODE_BACK};
        Transparency transparency{TRANSPARENCY_DISABLED};
        float        alphaScissor{0.1f};
        // The material parameters will be written in GPU memory next frame
        uint32_t     dirty;

    public:
        inline bool _isDirty() const { return dirty > 0; }
        void _setDirty();
        inline void _clearDirty() { this->dirty--; }
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
            TextureChannel           channel{TEXTURE_CHANNEL_NONE};
            vec2                     offset{0.0f, 0.0f};
            vec2                     scale{1.0f, 1.0f};
        };

        /**
         * Creates a StandardMaterial with default parameters
         */
        explicit StandardMaterial(const string &name = "StandardMaterial");

        /**
         * Returns the material's base color.
         */
        [[nodiscard]] inline const Color &getAlbedoColor() const { return albedoColor; }

        /**
         * Sets the material's base color.
         */
        inline void setAlbedoColor(const Color &color) { albedoColor = color; }

        /**
         * Returns the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        [[nodiscard]] inline const TextureInfo &getAlbedoTexture() const { return albedoTexture; }

        /**
         * Sets the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        void setAlbedoTexture(const TextureInfo &texture);

        /**
         * Return the specular texture
         */
        [[nodiscard]] inline const TextureInfo &getSpecularTexture() const { return specularTexture; }

        /**
         * Sets the specular texture
         */
        void setSpecularTexture(const TextureInfo &texture);

        /**
         * Return the normal texture
         */
        [[nodiscard]] inline const TextureInfo &getNormalTexture() const { return normalTexture; }

        /**
         * Sets the normal texture
         */
        void setNormalTexture(const TextureInfo &texture);

        [[nodiscard]] inline float getMetallicFactor() const { return metallicFactor; }

        void setMetallicFactor(float metallic);

        [[nodiscard]] inline const TextureInfo& getMetallicTexture() const { return metallicTexture; }

        void setMetallicTexture(const TextureInfo &texture);

        [[nodiscard]] inline float getRoughnessFactor() const { return roughnessFactor; }

        void setRoughnessFactor(float roughness);

        [[nodiscard]] inline const TextureInfo& getRoughnessTexture() const { return roughnessTexture; }

        void setRoughnessTexture(const TextureInfo &texture);

        [[nodiscard]] inline const TextureInfo& getAmbientOcclusionTexture() const { return ambientOcclusionTexture; }

        void setAmbientOcclusionTexture(const TextureInfo &texture);

        [[nodiscard]] inline const TextureInfo& getEmissiveTexture() const { return emissiveTexture; }

        [[nodiscard]] inline vec3 getEmissiveFactor() const { return emissiveFactor; }

        void setEmissiveFactor(const vec3& emissive);

        void setEmissiveTexture(const TextureInfo& texture);

        [[nodiscard]] inline float getEmissiveStrength() const { return emissiveStrength; }

        void setEmissiveStrength(float emissive);


    private:
        Color        albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
        TextureInfo  albedoTexture{};
        float        metallicFactor{0.0f};
        TextureInfo  metallicTexture{.channel = TEXTURE_CHANNEL_BLUE}; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_pbrmetallicroughness_metallicroughnesstexture
        float        roughnessFactor{1.0f};
        TextureInfo  roughnessTexture{.channel = TEXTURE_CHANNEL_GREEN}; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_pbrmetallicroughness_metallicroughnesstexture
        vec3         emissiveFactor{0.0f}; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_emissivefactor
        float        emissiveStrength{1.0f}; // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_emissive_strength/README.md
        TextureInfo  emissiveTexture; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_emissivetexture
        TextureInfo  ambientOcclusionTexture{.channel =TEXTURE_CHANNEL_RED}; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_occlusiontexture
        TextureInfo  specularTexture{};
        TextureInfo  normalTexture{}; // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_normaltexture
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
        explicit ShaderMaterial(string        fragShaderFileName,
                                string        vertShaderFileName = "",
                                const string &name               = "ShaderMaterial");

        /**
         * Returns the fragment shader file path, relative to the application directory
         */
        [[nodiscard]] inline const string &getFragFileName() const { return fragFileName; }

        /**
         * Returns the vertex shader file path, relative to the application directory
         */
        [[nodiscard]] inline const string &getVertFileName() const { return vertFileName; }

        /**
         * Sets a parameter value
         */
        void setParameter(const int index, const vec4 value);

        /**
         * Returns a parameter value
         */
        [[nodiscard]] inline vec4 getParameter(const int index) const { return parameters[index]; }

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
        /**
         * Returns a given outline material
         */
        static shared_ptr<ShaderMaterial> &get(int index);

        /**
         * Adds an outline material.
         */
        static void add(const shared_ptr<ShaderMaterial> &material);

    private:
        static vector<shared_ptr<ShaderMaterial>> materials;

    public:
        static vector<shared_ptr<ShaderMaterial>> &_all() {
            return materials;
        }

        static void _initialize();
    };

}
