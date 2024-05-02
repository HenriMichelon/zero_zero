#include "z0/nodes/skybox.h"

namespace z0 {

    Skybox::Skybox(const string& filename, const string& fileext, const string& nodeName):
        Node{nodeName}{
        cubemap = Cubemap::loadFromFile(filename, fileext);
    }

    void Skybox::_onEnterScene() {
        //Application::getViewport()._setSkyBox(*this);
        Node::_onEnterScene();
    }

}