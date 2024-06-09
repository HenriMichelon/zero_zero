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

}
