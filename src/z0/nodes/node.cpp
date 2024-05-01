#include "z0/nodes/node.h"
#include "z0/application.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <utility>

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(const string& nodeName):
        name{nodeName},
        id{currentId++}   {
        replace(name.begin(), name.end(),  '/', '_');
        localTransform = mat4 {1.0};
        updateTransform(mat4{1.0f});
    }

    Node::Node(const Node& orig): id{currentId++} {
        name = orig.name;
        parent = orig.parent;
        localTransform = orig.localTransform;
        worldTransform = orig.worldTransform;
        processMode = orig.processMode;
    }

    void Node::setPosition(vec3 pos) {
        localTransform[3] = vec4(pos, 1.0f);
        updateTransform(mat4{1.0f});
    }

    void Node::updateTransform(const mat4& parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::updateTransform() {
        auto parentMatrix = parent == nullptr ? mat4{1.0f} : parent->worldTransform;
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->updateTransform(worldTransform);
        }
    }

    void Node::rotateX(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_X);
        updateTransform();
    }

    void Node::rotateY(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Y);
        updateTransform();
    }

    void Node::rotateZ(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Z);
        updateTransform();
    }

    void Node::_onReady() {
        inReady = true;
        onReady();
        inReady = false;
    }

    void Node::addChild(const shared_ptr<Node>& child) {
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
        if (inReady) child->_onReady();
        Application::get().addNode(child);
    }

    void Node::printTree(ostream& out, int tab) {
        for (int i = 0; i < (tab*2); i++) {
            out << " ";
        }
        out << toString() << std::endl;
        for (auto& child: children) child->printTree(out, tab+1);
    }

    bool Node::isProcessed() const {
        const auto paused = Application::get().isPaused();
        auto mode = processMode;
        if ((parent == nullptr) && (mode == PROCESS_MODE_INHERIT)) mode = PROCESS_MODE_PAUSABLE;
        return ((mode == PROCESS_MODE_INHERIT) && (parent->isProcessed())) ||
               (!paused && (mode == PROCESS_MODE_PAUSABLE)) ||
               (paused && (mode == PROCESS_MODE_WHEN_PAUSED)) ||
               (mode == PROCESS_MODE_ALWAYS);
    }

}