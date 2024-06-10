#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#endif

namespace z0 {

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

    unique_ptr<OutlineMaterials> OutlineMaterials::instance;

    void OutlineMaterials::create() {
        instance = make_unique<OutlineMaterials>();
    }
    
    OutlineMaterials& OutlineMaterials::get() {
        if (instance == nullptr) die("OutlineMaterial singleton not created");
        return *instance;
    }

    OutlineMaterials::OutlineMaterials() {
        if (instance != nullptr) die("OutlineMaterial singleton already exists");
        auto outlineMaterial = make_shared<ShaderMaterial>("outline.frag");
        outlineMaterial->setParameter(0, {1.0f, 0.9f, 0.2f, 1.0f});        
        outlineMaterial->setParameter(1, vec4{0.1f});
        add(outlineMaterial);
    }

    int OutlineMaterials::count() {
        return materials.size();
    }

    shared_ptr<ShaderMaterial>& OutlineMaterials::get(int index) {
        assert(index < materials.size());
        return materials[index];
    }

    void OutlineMaterials::add(const shared_ptr<ShaderMaterial>&material) {
        materials.push_back(material);
    }

}
