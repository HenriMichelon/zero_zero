#include <z0/input.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/mesh_instance.h>

#include "ui.h"

void UIMainScene::onReady() {
    auto camera = make_shared<Camera>();
    addChild(camera);

    const vector<Vertex> vertices {
            {.position = {-0.5, 0.5, 0.0}},
            {.position = {0.5, 0.5, 0.0}},
            {.position = {0.5, -0.5, 0.0}},
            {.position = {-0.5, -0.5, 0.0}},
    };
    const vector<uint32_t> indices {
            0, 1, 2, 2, 3, 0
    };

    const vector<shared_ptr<Surface>> surfaces1 {
            make_shared<Surface>(0, indices.size())
    };
    auto mesh1 = make_shared<Mesh>(vertices, indices, surfaces1);
    auto material1 = make_shared<StandardMaterial>();
    material1->setAlbedoColor(Color(vec4{0.5, 0.5, 0.5, 0.5}));
    material1->setCullMode(CULLMODE_FRONT);
    mesh1->setSurfaceMaterial(0, material1);
    auto triangle1 = make_shared<MeshInstance>(mesh1);
    triangle1->setPosition({0.0, 0.0, -1.0});
    //addChild(triangle1);
}

//TODO : , mouse inputevent, mouse buttons,
void UIMainScene::onProcess(float alpha) {
    if (Input::isMouseButtonJustReleased(MOUSE_BUTTON_LEFT)) {
        cout << "MOUSE LEFT" << endl;
    }
}