#include "triangle.h"
#include <z0/application.h>

void MainScene::onReady() {
    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 1.0f});
    addChild(camera);
    addChild(make_shared<Triangle>());

    printTree(cout);
}

void Triangle::onReady() {
    const vector<Vertex> vertices {
            {.position = {0.0, 0.5, 0.0}, .uv = {0.5, 0.25}},
            {.position = {0.5, -0.5, 0.0}, .uv = {0.75, 0.75}},
            {.position = {-0.5, -0.5, 0.0f}, .uv = {0.25, 0.75}},
    };
    const vector<uint32_t> indices {
            0, 1, 2
    };
    const vector<shared_ptr<Surface>> surfaces {
            make_shared<Surface>(0, indices.size())
    };
    triangle = make_shared<MeshInstance>(make_shared<Mesh>(vertices, indices, surfaces));
    addChild(triangle);
}

void Triangle::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    triangle->rotateY(angle);
}