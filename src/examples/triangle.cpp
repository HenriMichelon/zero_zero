#include "triangle.h"
#include <z0/application.h>
#include <z0/nodes/mesh_instance.h>

void Main::onReady() {
    cout << *this << ".onReady" << endl;
    cout << Application::get().getWindow() << endl;

    auto camera = make_shared<Camera>();
    camera->setPosition({0.0f, 0.0f, 0.0f});
    addChild(camera);

    // https://vulkan-tutorial.com/Vertex_buffers/Index_buffer
    const vector<Vertex> vertices {
            {.position = {-0.5f, -0.5f, 0.0f}},
            {.position = {0.5f, -0.5f, 0.0f}},
            {.position = {0.5f, 0.5f, 0.0f}},
            {.position = {-0.5f, 0.5f, 0.0f}}
    };
    const vector<uint32_t> indices = {
            0, 1, 2//, 2, 3, 0
    };
    const vector<shared_ptr<Surface>>& surfaces {
            make_shared<Surface>(0, indices.size())
    };
    auto triangleMesh = make_shared<Mesh>(vertices, indices, surfaces);
    auto triangle = make_shared<MeshInstance>(triangleMesh);
    addChild(triangle);

    printTree(cout);
}
