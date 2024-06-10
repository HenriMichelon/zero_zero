#pragma once

namespace z0 {

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

    class ShaderMaterial: public Material {
    public:
        static const int MAX_PARAMETERS = 4;
        
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

    class OutlineMaterials {
    public:
        OutlineMaterials();

        static void create();
        static OutlineMaterials& get();
        int count();
        shared_ptr<ShaderMaterial>& get(int index);
        void add(const shared_ptr<ShaderMaterial>&);
        vector<shared_ptr<ShaderMaterial>>& all() { return materials; }
        
    private:
        static unique_ptr<OutlineMaterials> instance;
        vector<shared_ptr<ShaderMaterial>> materials;
    };

}