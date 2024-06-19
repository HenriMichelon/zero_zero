#pragma once

namespace z0 {

    /**
     * Base class for all materials
     */
    class Material: public Resource {
    public:
        CullMode getCullMode() const { return cullMode;}
        void setCullMode(CullMode mode) {cullMode = mode;}

        Transparency getTransparency() const { return transparency;}
        void setTransparency(Transparency transparencyMode) {transparency = transparencyMode;}

        float getAlphaScissor() const { return alphaScissor;}
        void setAlphaScissor(float scissor) {alphaScissor = scissor;}

    protected:
        explicit Material(const string& name): Resource(name) {}

    private:
        CullMode cullMode { CULLMODE_BACK };
        Transparency transparency { TRANSPARENCY_DISABLED };
        float alphaScissor { 0.1 };
    };

    /**
     * Simple albedo/specular material
     */
    class StandardMaterial: public Material {
    public:
        explicit StandardMaterial(const string& name = "StandardMaterial"): Material(name) {}

        const Color &getAlbedoColor() const { return albedoColor;}
        void setAlbedoColor(const Color &color) { albedoColor = color;}

        const shared_ptr<ImageTexture> &getAlbedoTexture() const { return albedoTexture;}
        void setAlbedoTexture(const shared_ptr<ImageTexture> &texture) { albedoTexture = texture; }

        const shared_ptr<ImageTexture> &getSpecularTexture() const { return specularTexture;}
        void setSpecularTexture(const shared_ptr<ImageTexture> &texture) {specularTexture = texture;}

        const shared_ptr<ImageTexture> &getNormalTexture() const {return normalTexture;}
        void setNormalTexture(const shared_ptr<ImageTexture> &texture) {normalTexture = texture;}

    private:
        Color                      albedoColor {0.8f, 0.3f, 0.5f, 1.0f };
        shared_ptr<ImageTexture>   albedoTexture {nullptr};
        shared_ptr<ImageTexture>   specularTexture {nullptr};
        shared_ptr<ImageTexture>   normalTexture {nullptr};
    };

    /**
     * Shader based material
     */
    class ShaderMaterial: public Material {
    public:
        static const int MAX_PARAMETERS = 4;
        
        explicit ShaderMaterial(const shared_ptr<ShaderMaterial>&);
        explicit ShaderMaterial(string fragShaderFileName, 
                                string vertShaderFileName = "",
                                const string& name = "ShaderMaterial");

        const string& getFragFileName() const { return fragFileName; }
        const string& getVertFileName() const { return vertFileName; }
        void setParameter(int index, vec4 value);
        vec4 getParameter(int index);

    private:
        const string fragFileName;
        const string vertFileName;
        vec4 parameters[MAX_PARAMETERS];
    };

    /**
     * Singleton for all the materials used by the SceneRenderer to oultine meshes
     */
    class OutlineMaterials {
    public:
        static shared_ptr<ShaderMaterial>& get(int index);
        static void add(const shared_ptr<ShaderMaterial>&);
        
    private:
        static vector<shared_ptr<ShaderMaterial>> materials;

    public:
        static vector<shared_ptr<ShaderMaterial>>& _all();
        static void _initialize();
    };

}