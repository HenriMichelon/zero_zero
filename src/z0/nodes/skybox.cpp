#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/cubemap.h"
#include "z0/nodes/skybox.h"
#endif

namespace z0 {

    Skybox::Skybox(const string& filename, const string& fileext):
        Node{filename}{
        cubemap = Cubemap::loadFromFile(filename, fileext);
    }

    Skybox::Skybox(const string& filename):
        Node{filename}{
        cubemap = Cubemap::loadFromFile(filename);
    }

    void Skybox::setCubemapFromFile(const string& filename) {
        cubemap = Cubemap::loadFromFile(filename);
    }

    void Skybox::setProperty(const string&property, const string& value) {
        Node::setProperty(property, value);
        if (property == "cubemap_file") {
            cubemap = Cubemap::loadFromFile(value);
        }
    }

}