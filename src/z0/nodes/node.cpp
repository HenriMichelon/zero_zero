#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/application.h"
#endif

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(string nodeName):
        name{std::move(nodeName)},
        id{currentId++}{
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

    void Node::translate(vec3 localOffset) {
        // current orientation * velocity
        vec3 worldTranslation =  toQuat(mat3(localTransform)) * localOffset;
        setPosition(getPosition() + worldTranslation);
    }

    void Node::setPosition(vec3 pos) {
        localTransform[3] = vec4(pos, 1.0f);
        updateTransform(mat4{1.0f});
    }

    void Node::setPositionGlobal(vec3 pos) {
        if (parent == nullptr) {
            setPosition(pos);
            return;
        }
        localTransform[3] = inverse(parent->worldTransform) * vec4{pos, 1.0};
        updateTransform();
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

    void Node::setRotation(quat quater) {
        vec3 scale, translation, skew;
        vec4 perspective;
        quat orientation;
        // Decompose the original matrix to extract translation, rotation (orientation), and scale
        decompose(localTransform, scale, orientation, translation, skew, perspective);
        // Create a rotation matrix from the new quaternion
        mat4 rotationMatrix = toMat4(quater);
        // Reconstruct the transformation matrix with the new rotation, preserving the original translation and scale
        localTransform = glm::translate(mat4(1.0f), translation)
                         * rotationMatrix
                         * glm::scale(mat4(1.0f), scale);
        updateTransform();
    }

    vec3 Node::getRotation() const {
        return eulerAngles(toQuat(mat3(localTransform)));
    };

    void Node::setRotationX(float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::setScale(float scale) {
        setScale(vec3{scale, scale, scale});
    }

    void Node::setScale(glm::vec3 scale) {
        localTransform = glm::scale(localTransform, scale);
        updateTransform();
    }

    glm::vec3 Node::getScale() const {
        vec3 scale;
        quat rotation;
        vec3 translation;
        vec3 skew;
        vec4 perspective;
        decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    void Node::_onReady() {
        inReady = true;
        onReady();
        inReady = false;
    }

    vec3 Node::toGlobal(vec3 local) const {
        return vec3{worldTransform * vec4{local, 1.0f}};
    }

    shared_ptr<Node> Node::getChild(const string& name) const {
        auto it = std::find_if(children.begin(), children.end(), [name](std::shared_ptr<Node> elem) {
            return elem->name == name;
        });
        return it == children.end() ? nullptr : *it;
    }

    shared_ptr<Node> Node::getNode(const string& path) const {
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

    /*bool Node::isParent(const shared_ptr<Node>& node) const {
        if (node.get() == parent) { return true; }
        for(const auto& child : children) {
            if (child->isParent(node)) { return true; }
        }
        return false;
    }*/

    bool Node::addChild(const shared_ptr<Node>& child) {
        if (haveChild(child, false)) return false;
        children.push_back(child);
        child->parent = this;
        child->updateTransform(worldTransform);
        if (inReady || (parent != nullptr)) { child->_onReady(); }
        if (addedToScene) { Application::get()._addNode(child); }
        return true;
    }

    bool Node::removeChild(const shared_ptr<Node>& node) {
        if (!haveChild(node, false)) return false;
        node->parent = nullptr;
        if (node->addedToScene) { Application::get()._removeNode(node); }
        children.remove(node);
        return true;
    }

    void Node::removeAllChildren() {
        for(const auto& node : children) {
            node->parent = nullptr;
            if (node->addedToScene) { Application::get()._removeNode(node); }
        }
        children.clear();
    }

    bool Node::haveChild(const shared_ptr<z0::Node> &child, bool recursive) const {
        if (recursive) {
            if (haveChild(child, false)) return true;
            for(const auto& node : children) {
                if (node->haveChild(child, true)) return true;
            }
            return false;
        } else {
            return find(children.begin(), children.end(), child) != children.end();
        }
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

    void Node::printTree(int tab) const {
        stringstream sstream;
        for (int i = 0; i < (tab*2); i++) {
            sstream << " ";
        }
        sstream << " " << toString();
        log(sstream.str());
        for (auto& child: children) child->printTree(tab+1);
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

    Application& Node::app() {
        return Application::get();
    }

}