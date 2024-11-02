/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0;

import :Cubemap;
import :Skybox;

namespace z0 {

    Skybox::Skybox(const string &filename, const string &fileext):
        Node{filename, SKYBOX} {
        cubemap = Cubemap::loadFromFile(filename, fileext);
    }

    Skybox::Skybox(const string &filename):
        Node{filename, SKYBOX} {
        if (filename.ends_with(".hdr")) {
            cubemap = EnvironmentCubemap::loadFromHDRi(filename);
        } else {
            cubemap = Cubemap::loadFromFile(filename);
        }
    }

    void Skybox::setCubemapFromFile(const string &filename) {
        cubemap = Cubemap::loadFromFile(filename);
    }

    void Skybox::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "cubemap_file") { cubemap = Cubemap::loadFromFile(value); }
    }

}
