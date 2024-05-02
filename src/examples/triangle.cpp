#include "triangle.h"
#include <z0/application.h>
#include <z0/input.h>

void TriangleMainScene::onReady() {
    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 2.0f});
    addChild(camera);
    addChild(make_shared<Triangle>());

    printTree(cout);
}

//TODO shader material + skybox
void Triangle::onReady() {
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
    material1 = make_shared<StandardMaterial>();
    material1->setAlbedoColor(Color(vec3{0.5, 0.5, 0.5}));
    material1->setCullMode(CULLMODE_DISABLED);
    mesh1->setSurfaceMaterial(0, material1);
    triangle1 = make_shared<MeshInstance>(mesh1);
    triangle1->setPosition({1.0, 0.0, 0.0});
    addChild(triangle1);

    const vector<shared_ptr<Surface>> surfaces2 {
            make_shared<Surface>(0, indices.size())
    };
    auto mesh2 = make_shared<Mesh>(vertices, indices, surfaces2);
    material2 = make_shared<ShaderMaterial>("examples/uv_gradient.frag");
    material2->setCullMode(CULLMODE_DISABLED);
    mesh2->setSurfaceMaterial(0, material2);
    triangle2 = make_shared<MeshInstance>(mesh2);
    triangle2->setPosition({-1.0, 0.0, 0.0});
    addChild(triangle2);
}

void Triangle::onPhysicsProcess(float delta) {
    auto angle = delta * radians(90.0f) / 2;
    triangle1->rotateY(angle);
    triangle2->rotateY(-angle);
}

void Triangle::onProcess(float alpha) {
    if (Input::isKeyJustPressed(KEY_ENTER)) {
        if (triangle1->getMesh()->getSurfaceMaterial(0).get() == material1.get()) {
            triangle1->getMesh()->setSurfaceMaterial(0, material2);
            triangle2->getMesh()->setSurfaceMaterial(0, material1);
        } else {
            triangle1->getMesh()->setSurfaceMaterial(0, material1);
            triangle2->getMesh()->setSurfaceMaterial(0, material2);

        }
    }
}