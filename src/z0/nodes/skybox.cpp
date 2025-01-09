/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.Skybox;

import z0.resources.Cubemap;

namespace z0 {

    Skybox::Skybox(const string &filename, const string &fileext):
        Node{filename, SKYBOX} {
        cubemap = Cubemap::load(filename, fileext);
    }

    Skybox::Skybox(const string &filename):
        Node{filename, SKYBOX} {
        if (filename.ends_with(".hdr")) {
            cubemap = EnvironmentCubemap::loadFromHDRi(filename);
        } else {
            cubemap = Cubemap::load(filename);
        }
    }

    void Skybox::setCubemapFromFile(const string &filename) {
        cubemap = Cubemap::load(filename);
    }

    void Skybox::setProperty(const string &property, const string &value) {
        Node::setProperty(property, value);
        if (property == "cubemap_file") { cubemap = Cubemap::load(value); }
    }

    shared_ptr<Node> Skybox::duplicateInstance() {
        return make_shared<Skybox>(*this);
    }

}
