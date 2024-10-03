module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

module z0;

import :Node;
import :Application;

namespace z0 {

    Node::id_t Node::currentId = 0;

    Node::Node(const Node &orig):
        id{currentId++} {
        name           = orig.name;
        localTransform = orig.localTransform;
        worldTransform = orig.worldTransform;
        processMode    = orig.processMode;
        type           = orig.type;
    }

    Node::Node(const string &nodeName, Type type):
        type{type},
        name{std::move(nodeName)},
        id{currentId++} {
        replace(name.begin(), name.end(), '/', '_');
        localTransform = mat4{1.0};
        _updateTransform(mat4{1.0f});
    }

    vec3 Node::toGlobal(const vec3 local) const {
        return vec3{worldTransform * vec4{local, 1.0f}};
    }

    void Node::setPosition(const vec3 position) {
        localTransform[3] = vec4{position, 1.0f};
        _updateTransform();
    }

    vec3 Node::getRotation() const { return eulerAngles(toQuat(mat3(localTransform))); }

    void Node::translate(const vec3 localOffset) {
        // current orientation * velocity
        const vec3 worldTranslation = toQuat(mat3(localTransform)) * localOffset;
        setPosition(getPosition() + worldTranslation);
    }

    void Node::setPositionGlobal(const vec3 position) {
        if (parent == nullptr) {
            setPosition(position);
            return;
        }
        localTransform[3] = inverse(parent->worldTransform) * vec4{position, 1.0};
        _updateTransform();
    }

    void Node::setScale(const vec3 scale) {
        localTransform = glm::scale(localTransform, scale);
        _updateTransform();
    }

    void Node::setScale(const float scale) {
        setScale(vec3{scale, scale, scale});
    }

    vec3 Node::getScale() const {
        vec3 scale;
        quat rotation;
        vec3 translation;
        vec3 skew;
        vec4 perspective;
        decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    bool Node::haveChild(const shared_ptr<Node> &child, const bool recursive) const {
        if (recursive) {
            if (haveChild(child, false))
                return true;
            for (const auto &node : children) {
                if (node->haveChild(child, true))
                    return true;
            }
            return false;
        }
        return find(children.begin(), children.end(), child) != children.end();
    }

    shared_ptr<Node> Node::getChild(const string &name) const {
        auto it = std::find_if(children.begin(),
                               children.end(),
                               [name](std::shared_ptr<Node> elem) {
                                   return elem->name == name;
                               });
        return it == children.end() ? nullptr : *it;
    }

    shared_ptr<Node> Node::getNode(const string &path) const {
        size_t pos = path.find('/');
        if (pos != std::string::npos) {
            auto child = getChild(path.substr(0, pos));
            if (child != nullptr) {
                return child->getNode(path.substr(pos + 1));
            }
            return nullptr;
        }
        return getChild(path);
    }

    void Node::setRotation(const quat quater) {
        vec3 scale, translation, skew;
        vec4 perspective;
        quat orientation;
        // Decompose the original matrix to extract translation, rotation (orientation), and scale
        decompose(localTransform, scale, orientation, translation, skew, perspective);
        // Create a rotation matrix from the new quaternion
        mat4 rotationMatrix = toMat4(quater);
        // Reconstruct the transformation matrix with the new rotation, preserving the original translation and scale
        localTransform = glm::translate(mat4{1.0f}, translation)
                * rotationMatrix
                * glm::scale(mat4{1.0f}, scale);
        _updateTransform();
    }

    void Node::rotateX(const float angle) {
        localTransform = rotate(localTransform, angle, AXIS_X);
        _updateTransform();
    }

    void Node::rotateY(const float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Y);
        _updateTransform();
    }

    void Node::rotateZ(const float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Z);
        _updateTransform();
    }

    void Node::setRotationX(const float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(const float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(const float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::printTree(const int tab) const {
        stringstream sstream;
        for (int i = 0; i < (tab * 2); i++) {
            sstream << " ";
        }
        sstream << " " << toString();
        log(sstream.str());
        for (auto &child : children)
            child->printTree(tab + 1);
    }

    shared_ptr<Node> Node::duplicate() {
        shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for (const auto &child : children) {
            dup->addChild(child->duplicate());
        }
        dup->id   = currentId++;
        dup->name = name;
        return dup;
    }

    vec3 Node::getRightVector() const {
        return normalize(mat3{worldTransform} * AXIS_X);
    }

    void Node::killTween(const shared_ptr<Tween> &tween) {
        if (tween != nullptr) {
            tween->_kill();
            tweens.remove(tween);
        }
    }

    void Node::setProperty(const string &property, const string &value) {
        if (property == "position") {
            setPosition(to_vec3(value));
        } else if (property == "rotation") {
            const auto rot = to_vec3(value);
            setRotation(vec3{radians(rot.x), radians(rot.y), radians(rot.z)});
        } else if (property == "process_mode") {
            auto v = to_lower(value);
            if (v == "inherit") {
                setProcessMode(PROCESS_MODE_INHERIT);
            } else if (v == "pausable") {
                setProcessMode(PROCESS_MODE_PAUSABLE);
            } else if (v == "when_paused") {
                setProcessMode(PROCESS_MODE_WHEN_PAUSED);
            } else if (v == "always") {
                setProcessMode(PROCESS_MODE_ALWAYS);
            } else if (v == "disabled") {
                setProcessMode(PROCESS_MODE_DISABLED);
            }
        }
    }

    void Node::_physicsUpdate(const float delta) {
        for (auto it = tweens.begin(); it != tweens.end();) {
            if ((*it)->update(delta)) {
                it = tweens.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Node::_updateTransform(const mat4 &parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (const auto &child : children) {
            child->_updateTransform(worldTransform);
        }
    }

    void Node::_updateTransform() {
        const auto parentMatrix = parent == nullptr ? mat4{1.0f} : parent->worldTransform;
        _updateTransform(parentMatrix);
    }

    bool Node::addChild(const shared_ptr<Node> child) {
        if (haveChild(child, false)) { return false; }
        if (child->parent != nullptr) {
            die("remove child from parent first");
        }
        child->parent = this;
        children.push_back(child);
        child->_updateTransform(worldTransform);
        if ((inReady || (parent != nullptr)) && isProcessed()) { child->_onReady(); }
        if (addedToScene) { Application::get()._addNode(child); }
        return true;
    }

    bool Node::removeChild(const shared_ptr<Node> &node) {
        if (!haveChild(node, false)) { return false; }
        node->parent = nullptr;
        if (node->addedToScene) { Application::get()._removeNode(node); }
        children.remove(node);
        return true;
    }

    list<shared_ptr<Node>>::const_iterator Node::removeChild(const list<shared_ptr<Node>>::const_iterator &it) {
        auto &node   = *it;
        node->parent = nullptr;
        if (node->addedToScene) { Application::get()._removeNode(node); }
        return children.erase(it);
    }

    void Node::removeAllChildren() {
        for (const auto &node : children) {
            node->parent = nullptr;
            if (node->addedToScene) { Application::get()._removeNode(node); }
        }
        children.clear();
    }

    bool Node::isProcessed() const {
        const auto paused = Application::get().isPaused();
        auto       mode   = processMode;
        if ((parent == nullptr) && (mode == PROCESS_MODE_INHERIT))
            mode = PROCESS_MODE_PAUSABLE;
        return ((mode == PROCESS_MODE_INHERIT) && (parent->isProcessed())) ||
                (!paused && (mode == PROCESS_MODE_PAUSABLE)) ||
                (paused && (mode == PROCESS_MODE_WHEN_PAUSED)) ||
                (mode == PROCESS_MODE_ALWAYS);
    }

    shared_ptr<Node> Node::duplicateInstance() {
        return make_shared<Node>(*this);
    }

    void Node::_onReady() {
        inReady = true;
        onReady();
        inReady = false;
    }

    Application &Node::app() const {
        return Application::get();
    }
}
