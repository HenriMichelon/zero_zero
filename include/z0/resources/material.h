#pragma once

namespace z0 {

    /**
     * Base class for all materials of models surfaces
     */
    class Material: public Resource {
    public:
        /**
         * Returns the cull mode.
         */
        inline CullMode getCullMode() const { return cullMode; }

        /**
         * Sets the CullMode.
         * Determines which side of the triangle to cull depending on whether the triangle faces towards or away from the camera.
         */
        void setCullMode(CullMode mode) { cullMode = mode; }

        /**
         * Returns the transparency mode
         */
        inline Transparency getTransparency() const { return transparency; }

        /**
         * Sets the transparency mode
         */
        void setTransparency(Transparency transparencyMode) { transparency = transparencyMode; }

        /**
         * Returns the alpha scissor threshold value
         */
        inline float getAlphaScissor() const { return alphaScissor; }

        /**
         * Sets the alpha scissor threshold value
         * Threshold at which the alpha scissor will discard values. 
         * Higher values will result in more pixels being discarded. 
         * If the material becomes too opaque at a distance, try increasing this value. 
         * If the material disappears at a distance, try decreasing this value.
         */
        void setAlphaScissor(float scissor) { alphaScissor = scissor; }

    protected:
        explicit Material(const string& name): Resource(name) {}

    private:
        CullMode        cullMode     { CULLMODE_BACK };
        Transparency    transparency { TRANSPARENCY_DISABLED };
        float           alphaScissor { 0.1f };
    };

    /**
     * Simple albedo/specular/normal material
     */
    class StandardMaterial: public Material {
    public:
        /**
         * Textures UV locals transforms
         */
        struct TextureTransform {
            vec2   offset  { 0.0f, 0.0f };
            vec2   scale   { 1.0f, 1.0f };
        };

        /**
         * Creates a StandardMaterial with default parameters
         */
        explicit StandardMaterial(const string& name = "StandardMaterial"): Material(name) {}

        /**
         * Returns the material's base color.
         */
        inline const Color &getAlbedoColor() const { return albedoColor; }

        /**
         * Sets the material's base color.
         */
        void setAlbedoColor(const Color &color) { albedoColor = color; }

        /**
         * Returns the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        inline const shared_ptr<ImageTexture>& getAlbedoTexture() const { return albedoTexture; }

        /**
         * Sets the albedo texture (texture to multiply by albedo color. Used for basic texturing of objects).
         */
        void setAlbedoTexture(const shared_ptr<ImageTexture> &texture) { albedoTexture = texture; }

        /**
         * Return the specular texture
         */
        inline const shared_ptr<ImageTexture>& getSpecularTexture() const { return specularTexture; }

        /**
         * Sets the specular texture
         */
        void setSpecularTexture(const shared_ptr<ImageTexture> &texture) {specularTexture = texture; }

        /**
         * Return the normal texture
         */
        inline const shared_ptr<ImageTexture>& getNormalTexture() const {return normalTexture; }

        /**
         * Sets the normal texture
         */
        void setNormalTexture(const shared_ptr<ImageTexture> &texture) {normalTexture = texture; }

        /**
         * Returns the texture's UV transform.
         */
        inline const shared_ptr<TextureTransform>& getTextureTransform() const { return textureTransform; }

        /**
         * Sets the texture's UV transform.
         */
        void setTextureTransform(TextureTransform transform) { textureTransform = make_shared<TextureTransform>(transform); }

    private:
        Color                           albedoColor     { 1.0f, 1.0f, 1.0f, 1.0f };
        shared_ptr<ImageTexture>        albedoTexture   { nullptr };
        shared_ptr<ImageTexture>        specularTexture { nullptr };
        shared_ptr<ImageTexture>        normalTexture   { nullptr };
        shared_ptr<TextureTransform>    textureTransform{ nullptr };
        
    };

    /**
     * Shader based material
     */
    class ShaderMaterial: public Material {
    public:
        /**
         * Maximum number of parameters of a ShaderMaterial
         */
        static const int MAX_PARAMETERS = 4;
        
        /**
         * Creates a ShaderMaterial by copy
         */
        explicit ShaderMaterial(const shared_ptr<ShaderMaterial>&);

        /**
         * Creates a ShaderMaterial
         * @param fragShaderFileName fragment shader file path, relative to the application directory
         * @param vertShaderFileName vertex shader file path, relative to the application directory
         * @param name Resource name
         */
        explicit ShaderMaterial(string fragShaderFileName, 
                                string vertShaderFileName = "",
                                const string& name = "ShaderMaterial");

        /**
         * Returns the fragment shader file path, relative to the application directory
         */
        const string& getFragFileName() const { return fragFileName; }

        /**
         * Returns the vertex shader file path, relative to the application directory
         */
        const string& getVertFileName() const { return vertFileName; }

        /**
         * Sets a parameter value
         */
        void setParameter(int index, vec4 value);

        /**
         * Returns a parameter value
         */
        vec4 getParameter(int index);

    private:
        const string fragFileName;
        const string vertFileName;
        vec4         parameters[MAX_PARAMETERS];
    };

    /**
     * Singleton for all the materials used by the SceneRenderer to oultine meshes
     */
    class OutlineMaterials {
    public:
        /**
         * Returns a given outline material
         */
        static shared_ptr<ShaderMaterial>& get(int index);

        /**
         * Adds a outline material.
         */
        static void add(const shared_ptr<ShaderMaterial>&);
        
    private:
        static vector<shared_ptr<ShaderMaterial>> materials;

    public:
        static vector<shared_ptr<ShaderMaterial>>& _all();
        static void _initialize();
    };

}