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

    shared_ptr<Node> Node::getChild(const string& name) {
        auto it = std::find_if(children.begin(), children.end(), [name](std::shared_ptr<Node> elem) {
            return elem->name == name;
        });
        return it == children.end() ? nullptr : *it;
    }

    shared_ptr<Node> Node::getNode(const string& path) {
        size_t pos = path.find('/');
        if (pos != std::string::npos) {
            auto child = getChild(path.substr(0, pos));
            if (child != nullptr) {
                return child->getNode(path.substr(pos + 1));
            }
            return nullptr;
        } else {
            return getChild(path);
        }
    }

    bool Node::addChild(const shared_ptr<Node>& child) {
        if (find(children.begin(), children.end(), child) != children.end()) return false;
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
        if (inReady) child->_onReady();
        Application::get().addNode(child);
        return true;
    }

    bool Node::removeChild(const shared_ptr<Node>& node) {
        if (find(children.begin(), children.end(), node) == children.end()) return false;
        children.remove(node);
        node->parent = nullptr;
        Application::get().removeNode(node);
        return true;
    }


    shared_ptr<Node> Node::duplicate() {
        shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for(auto&child : children) {
            dup->addChild(child->duplicate());
        }
        dup->id = currentId++;
        dup->name = name;
        return dup;
    }

    shared_ptr<Node> Node::duplicateInstance() {
        return make_shared<Node>(*this);
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