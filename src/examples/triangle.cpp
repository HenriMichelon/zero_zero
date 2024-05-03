#include "triangle.h"
#include "z0/nodes/skybox.h"
#include <z0/application.h>
#include <z0/input.h>

void TriangleMainScene::onReady() {
    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 2.0f});
    addChild(camera);

    auto skybox = make_shared<Skybox>("examples/textures/sky", ".jpg");
    addChild(skybox);

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

    const vector<shared_ptr<Surface>> surfaces1 {
            make_shared<Surface>(0, indices.size())
    };
    auto mesh1 = make_shared<Mesh>(vertices, indices, surfaces1);
    material1 = make_shared<StandardMaterial>();
    material1->setAlbedoColor(Color(vec4{0.5, 0.5, 0.5, 0.5}));
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
    material2->setTransparency(TRANSPARENCY_ALPHA);
    material2->setCullMode(CULLMODE_DISABLED);
    material2->getParameters()[0] = 0.0;
    mesh2->setSurfaceMaterial(0, material2);
    triangle2 = make_shared<MeshInstance>(mesh2);
    triangle2->setPosition({-1.0, 0.0, 0.0});
    addChild(triangle2);
}

void Triangle::onPhysicsProcess(float delta) {
    if (rotate) {
        auto angle = delta * radians(90.0f) / 2;
        triangle1->rotateY(angle);
        triangle2->rotateY(-angle);
    }
    gradient += gradientSpeed * delta;
    // Ensure the color component remains within the range [0, 1]
    if (gradient > 1.0f) {
        gradient = 1.0f;
        gradientSpeed = -gradientSpeed;
    }
    if (gradient < 0.0f) {
        gradient = 0.0f;
        gradientSpeed = -gradientSpeed;
    }
    material2->getParameters()[0] = gradient;
}

void Triangle::onProcess(float alpha) {
    if (Input::isKeyJustPressed(KEY_ENTER)) {
        if (triangle1->getMesh()->getSurfaceMaterial(0).get() == material1.get()) {
            triangle1->getMesh()->setSurfaceMaterial(0, material2);
        } else {
            triangle1->getMesh()->setSurfaceMaterial(0, material1);
        }
    }
}

bool Triangle::onInput(InputEvent &inputEvent) {
    if (inputEvent.getType() == INPUT_EVENT_KEY) {
        auto& eventKey = dynamic_cast<InputEventKey&>(inputEvent);
        if ((eventKey.getKeyCode() == KEY_SPACE) && !eventKey.isPressed()) {
            rotate = !rotate;
            return true;
        }
    }
    return false;
}