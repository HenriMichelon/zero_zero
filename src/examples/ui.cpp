#include <z0/input.h>
#include <z0/nodes/camera.h>
#include <z0/nodes/mesh_instance.h>

#include "ui.h"

void UIMainScene::onReady() {
    auto camera = make_shared<Camera>();
    addChild(camera);

    const vector<Vertex> vertices {
            {.position = {0.0, 0.5, 0.0}, .uv = {0.5, 0.25}},
            {.position = {0.5, -0.5, 0.0}, .uv = {0.75, 0.75}},
            {.position = {-0.5, -0.5, 0.0f}, .uv = {0.25, 0.75}},
    };
    const vector<uint32_t> indices {
            0, 1, 2
    };

    const vector<shared_ptr<Surface>> surfaces1 {
            make_shared<Surface>(0, indices.size())
    };
    auto mesh1 = make_shared<Mesh>(vertices, indices, surfaces1);
    auto material1 = make_shared<StandardMaterial>();
    material1->setAlbedoColor(Color(vec4{0.5, 0.5, 0.5, 0.5}));
    material1->setCullMode(CULLMODE_DISABLED);
    mesh1->setSurfaceMaterial(0, material1);
    auto triangle1 = make_shared<MeshInstance>(mesh1);
    triangle1->setPosition({0.0, 0.0, -1.0});
    addChild(triangle1);
}

//TODO : key code + translate, mouse inputevent, mouse buttons,