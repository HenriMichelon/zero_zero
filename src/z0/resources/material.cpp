#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#endif

namespace z0 {

    ShaderMaterial::ShaderMaterial(const shared_ptr<ShaderMaterial>&orig):
        Material{orig->name},
        fragFileName{orig->fragFileName},
        vertFileName{orig->vertFileName} {
        for(int i = 0; i < MAX_PARAMETERS; i++) {
            parameters[i] = orig->parameters[i];
        }
    }

    ShaderMaterial::ShaderMaterial(string fragShaderFileName,
                                   string vertShaderFileName,
                                   const string& name): 
        Material{name}, 
        fragFileName{std::move(fragShaderFileName)},
        vertFileName{std::move(vertShaderFileName)}
        {}

    void ShaderMaterial::setParameter(int index, vec4 value) {
        parameters[index] = value;
    }
    

    vec4 ShaderMaterial::getParameter(int index) {
        return parameters[index];
    }

    vector<shared_ptr<ShaderMaterial>> OutlineMaterials::materials;

    void OutlineMaterials::_initialize() {
        auto outlineMaterial = make_shared<ShaderMaterial>("outline.frag", "outline.vert");
        outlineMaterial->setParameter(0, {0.0f, 0.0f, 0.0f, 1.0f});        
        outlineMaterial->setParameter(1, vec4{0.01f});
        add(outlineMaterial);
    }

    vector<shared_ptr<ShaderMaterial>>&  OutlineMaterials::_all() { 
        return materials; 
    }

    shared_ptr<ShaderMaterial>& OutlineMaterials::get(int index) {
        assert(index < materials.size());
        return materials[index];
    }

    void OutlineMaterials::add(const shared_ptr<ShaderMaterial>&material) {
        materials.push_back(material);
    }

}
