#include "z0/nodes/node.h"
#include "z0/application.h"

#include <algorithm>
#include <utility>

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(string nodeName): name{std::move(nodeName)}, id{currentId++}   {
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

    void Node::addChild(const shared_ptr<Node>& child) {
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
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
        bool paused = Application::get().isPaused();
        ProcessMode mode = processMode;
        if ((parent == nullptr) && (mode == PROCESS_MODE_INHERIT)) mode = PROCESS_MODE_PAUSABLE;
        return ((mode == PROCESS_MODE_INHERIT) && (parent->isProcessed())) ||
               (!paused && (mode == PROCESS_MODE_PAUSABLE)) ||
               (paused && (mode == PROCESS_MODE_WHEN_PAUSED)) ||
               (mode == PROCESS_MODE_ALWAYS);
    }

}