module;
#include <cassert>
#include "z0/libraries.h"

export module Z0:Material;

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
        void setCullMode(const CullMode mode) { cullMode = mode; }

        /**
         * Returns the transparency mode
         */
        [[nodiscard]] inline Transparency getTransparency() const { return transparency; }

        /**
         * Sets the transparency mode
         */
        void setTransparency(const Transparency transparencyMode) { transparency = transparencyMode; }

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
        void setAlphaScissor(const float scissor) { alphaScissor = scissor; }

    protected:
        explicit Material(const string& name): Resource(name) {
        }

    private:
        CullMode cullMode{CULLMODE_BACK};
        Transparency transparency{TRANSPARENCY_DISABLED};
        float alphaScissor{0.1f};
    };

    /**
     * Simple albedo/specular/normal material
     */
    class StandardMaterial : public Material {
    public:
        /**
         * Textures UV locals transforms
         */
        struct TextureTransform {
            vec2 offset{0.0f, 0.0f};
            vec2 scale{1.0f, 1.0f};
        };

        /**
         * Creates a StandardMaterial with default parameters
         */
        explicit StandardMaterial(const string& name = "StandardMaterial"): Material(name) {
        }

        /**
         * Returns the material's base color.
         */
        [[nodiscard]] inline const Color& getAlbedoColor() const { return albedoColor; }

        /**
         * Sets the material's base color.
         */
        void setAlbedoColor(const Color& color) { albedoColor = color; }

        /**
         * Returns the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        [[nodiscard]] inline const shared_ptr<ImageTexture>& getAlbedoTexture() const { return albedoTexture; }

        /**
         * Sets the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        void setAlbedoTexture(const shared_ptr<ImageTexture>& texture) { albedoTexture = texture; }

        /**
         * Return the specular texture
         */
        [[nodiscard]] inline const shared_ptr<ImageTexture>& getSpecularTexture() const { return specularTexture; }

        /**
         * Sets the specular texture
         */
        void setSpecularTexture(const shared_ptr<ImageTexture>& texture) { specularTexture = texture; }

        /**
         * Return the normal texture
         */
        [[nodiscard]] inline const shared_ptr<ImageTexture>& getNormalTexture() const { return normalTexture; }

        /**
         * Sets the normal texture
         */
        void setNormalTexture(const shared_ptr<ImageTexture>& texture) { normalTexture = texture; }

        /**
         * Returns the texture's UV transform.
         */
        [[nodiscard]] inline const shared_ptr<TextureTransform>& getTextureTransform() const {
            return textureTransform;
        }

        /**
         * Sets the texture's UV transform.
         */
        void setTextureTransform(const TextureTransform transform) {
            textureTransform = make_shared<TextureTransform>(transform);
        }

    private:
        Color albedoColor{1.0f, 1.0f, 1.0f, 1.0f};
        shared_ptr<ImageTexture> albedoTexture{nullptr};
        shared_ptr<ImageTexture> specularTexture{nullptr};
        shared_ptr<ImageTexture> normalTexture{nullptr};
        shared_ptr<TextureTransform> textureTransform{nullptr};
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
        explicit ShaderMaterial(const shared_ptr<ShaderMaterial>& orig):
            Material{orig->name},
            fragFileName{orig->fragFileName},
            vertFileName{orig->vertFileName} {
            for (int i = 0; i < MAX_PARAMETERS; i++) {
                parameters[i] = orig->parameters[i];
            }
        }

        /**
         * Creates a ShaderMaterial
         * @param fragShaderFileName fragment shader file path, relative to the application directory
         * @param vertShaderFileName vertex shader file path, relative to the application directory
         * @param name Resource name
         */
        explicit ShaderMaterial(string fragShaderFileName,
                                string vertShaderFileName = "",
                                const string& name = "ShaderMaterial"):
            Material{name},
            fragFileName{std::move(fragShaderFileName)},
            vertFileName{std::move(vertShaderFileName)} {
        }

        /**
         * Returns the fragment shader file path, relative to the application directory
         */
        [[nodiscard]] const string& getFragFileName() const { return fragFileName; }

        /**
         * Returns the vertex shader file path, relative to the application directory
         */
        [[nodiscard]] const string& getVertFileName() const { return vertFileName; }

        /**
         * Sets a parameter value
         */
        void setParameter(const int index, const vec4 value) {
            parameters[index] = value;
        }

        /**
         * Returns a parameter value
         */
        [[nodiscard]] vec4 getParameter(const int index) const {
            return parameters[index];
        }

    private:
        const string fragFileName;
        const string vertFileName;
        vec4 parameters[MAX_PARAMETERS]{};
    };

    /**
     * Singleton for all the materials used by the SceneRenderer to outline meshes
     */
    class OutlineMaterials {
    public:
        /**
         * Returns a given outline material
         */
        static shared_ptr<ShaderMaterial>& get(const int index) {
            assert(index < materials.size());
            return materials[index];
        }

        /**
         * Adds an outline material.
         */
        static void add(const shared_ptr<ShaderMaterial>& material) {
            materials.push_back(material);
        }

    private:
        static vector<shared_ptr<ShaderMaterial>> materials;

    public:
        static vector<shared_ptr<ShaderMaterial>>& _all(){
            return materials;
        }

        static void _initialize() {
            const auto outlineMaterial = make_shared<ShaderMaterial>("outline.frag", "outline.vert");
            outlineMaterial->setParameter(0, {0.0f, 0.0f, 0.0f, 1.0f});
            outlineMaterial->setParameter(1, vec4{0.01f});
            add(outlineMaterial);
        }
    };

    vector<shared_ptr<ShaderMaterial>> OutlineMaterials::materials;

}
