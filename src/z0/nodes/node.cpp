/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "z0/libraries.h"

module z0.nodes.Node;

import z0.Constants;
import z0.Application;
import z0.Tools;
import z0.Tween;

namespace z0 {

    Node::~Node() {
        // log("~", getName(), to_string(getId()));
    }

    Node::id_t Node::currentId = 1;

    Node::Node(const Node &orig):
        id{currentId++} {
        name           = orig.name;
        localTransform = orig.localTransform;
        worldTransform = orig.worldTransform;
        processMode    = orig.processMode;
        type           = orig.type;
    }

    Node::Node(const string &nodeName, const Type type):
        type{type},
        name{sanitizeName(nodeName)},
        id{currentId++} {
        localTransform = mat4{1.0};
        _updateTransform(mat4{1.0f});
    }

    string Node::sanitizeName(const string &name) {
        string newName = name;
        ranges::replace(newName, '/', '_');
        ranges::replace(newName, ':', '_');
        return newName;
    }

    vec3 Node::toGlobal(const vec3 local) const {
        return vec3{worldTransform * vec4{local, 1.0f}};
    }

    vec3 Node::toLocal(const vec3 global) const {
        return vec3{inverse(worldTransform) * localTransform * vec4{global, 1.0f}};
    }

    void Node::setPosition(const vec3 position) {
        localTransform[3] = vec4{position, 1.0f};
        _updateTransform();
    }

    vec3 Node::getRotation() const { return eulerAngles(getRotationQuaternion()); }

    void Node::translate(const vec3& localOffset) {
        localTransform = glm::translate(localTransform, localOffset);
        _updateTransform();
    }

    void Node::setPositionGlobal(const vec3& position) {
        if (parent == nullptr) {
            setPosition(position);
            return;
        }
        localTransform[3] = inverse(parent->worldTransform) * vec4{position, 1.0};
        _updateTransform();
    }

    void Node::setScale(const vec3& scale) {
        vec3 old_scale, translation, skew;
        vec4 perspective;
        quat orientation;
        decompose(localTransform, old_scale, orientation, translation, skew, perspective);
        localTransform = glm::translate(translation) *
                               mat4_cast(orientation) *
                               glm::scale(scale);
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
            if (haveChild(child, false)) {
                return true;
            }
            for (const auto &node : children) {
                if (node->haveChild(child, true))
                    return true;
            }
            return false;
        }
        return find(children.begin(), children.end(), child) != children.end();
    }

    // shared_ptr<Node> Node::getChild(const string &name) const {
    //     const auto it = std::find_if(children.begin(),
    //                                  children.end(),
    //                                  [name](std::shared_ptr<Node> elem) {
    //                                      return elem->name == name;
    //                                  });
    //     return it == children.end() ? nullptr : *it;
    // }

    // shared_ptr<Node> Node::findFirstChild(const string& name) const {
    //     for (const auto &node : children) {
    //         if (node->name == name) return node;
    //         if (const auto& found = node->findFirstChild(name)) return found;
    //     }
    //     return {nullptr};
    // }

    // shared_ptr<Node> Node::getChildByPath(const string &path) const {
    //     const size_t pos = path.find('/');
    //     if (pos != std::string::npos) {
    //         const auto child = getChild(path.substr(0, pos));
    //         if (child != nullptr) {
    //             return child->getChildByPath(path.substr(pos + 1));
    //         }
    //         return nullptr;
    //     }
    //     return getChild(path);
    // }

    void Node::setRotation(const vec3& rot) {
        setRotation(glm::quat(rot));
    }

    void Node::setRotation(const quat& quater) {
        vec3 scale, translation, skew;
        vec4 perspective;
        quat orientation;
        decompose(localTransform, scale, orientation, translation, skew, perspective);
        const mat4 rotationMatrix = toMat4(quater);
        localTransform = glm::translate(mat4{1.0f}, translation)
                * rotationMatrix
                * glm::scale(mat4{1.0f}, scale);
        _updateTransform();
    }

    // void Node::rotate(quat quater) {
    //     localTransform =  localTransform * toMat4(quater);
    //     _updateTransform();
    // }

    void Node::rotateX(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_X);
        _updateTransform();
    }

    void Node::rotateY(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Y);
        _updateTransform();
    }

    void Node::rotateZ(const float angle) {
        localTransform = glm::rotate(localTransform, angle, AXIS_Z);
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
        sstream << " " << toString() << " (" << TypeNames[type] << ") #" << getId();
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
        dup->id     = currentId++;
        dup->name   = name;
        return dup;
    }

    void Node::killTween(const shared_ptr<Tween> &tween) {
        if (tween != nullptr) {
            tween->_kill();
            tweens.remove(tween);
        }
    }

    void Node::setProperty(const string &property, const string &value) {
        if (property == "position") {
            setPositionGlobal(to_vec3(value));
        } else if (property == "rotation") {
            const auto rot = to_vec3(value);
            setRotation(vec3{radians(rot.x), radians(rot.y), radians(rot.z)});
        } else if (property == "scale") {
            setScale(to_vec3(value));
        } else if (property == "groups") {
            for(const auto groupName : split(value, ';')) {
                addToGroup(groupName.data());
            }
        } else if (property == "process_mode") {
            const auto v = to_lower(value);
            if (v == "inherit") {
                setProcessMode(ProcessMode::INHERIT);
            } else if (v == "pausable") {
                setProcessMode(ProcessMode::PAUSABLE);
            } else if (v == "when_paused") {
                setProcessMode(ProcessMode::WHEN_PAUSED);
            } else if (v == "always") {
                setProcessMode(ProcessMode::ALWAYS);
            } else if (v == "disabled") {
                setProcessMode(ProcessMode::DISABLED);
            }
        } else if (property == "visible") {
            setVisible(value == "true");
        } else if (property == "name") {
            setName(value);
        } else  if (property == "cast_shadows") {
            for (auto& child : _getChildren()) {
                child->setProperty(property, value);
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

    bool Node::addChild(const shared_ptr<Node> child, const bool async) {
        if (haveChild(child, false)) { return false; }
        if (child->parent != nullptr) {
            die("remove child from parent first");
        }
        child->parent = this;
        children.push_back(child);
        child->_updateTransform(worldTransform);
        child->_onReady();
        child->visible = visible && child->visible;
        if (addedToScene) { app()._addNode(child, async); }
        return true;
    }

    bool Node::removeChild(const shared_ptr<Node>& node, const bool async) {
        if (!haveChild(node, false)) { return false; }
        node->parent = nullptr;
        if (node->addedToScene) { app()._removeNode(node, async); }
        children.remove(node);
        return true;
    }

    // list<shared_ptr<Node>>::const_iterator Node::removeChild(const list<shared_ptr<Node>>::const_iterator &it) {
    //     auto &node   = *it;
    //     node->parent = nullptr;
    //     if (node->addedToScene) { app()._removeNode(node); }
    //     return children.erase(it);
    // }

    void Node::removeAllChildren(const bool async) {
        for (const auto &node : children) {
            node->parent = nullptr;
            if (node->addedToScene) { app()._removeNode(node, async); }
        }
        children.clear();
    }

    string Node::getPath() const {
        if (parent) {
            return parent->getPath() + "/" + getName();
        }
        return  "/" + name;
    }

    bool Node::isProcessed() const {
        const auto paused = app().isPaused();
        auto       mode   = processMode;
        if ((parent == nullptr) && (mode == ProcessMode::INHERIT))
            mode = ProcessMode::PAUSABLE;
        return ((mode == ProcessMode::INHERIT) && (parent->isProcessed())) ||
                (!paused && (mode == ProcessMode::PAUSABLE)) ||
                (paused && (mode == ProcessMode::WHEN_PAUSED)) ||
                (mode == ProcessMode::ALWAYS);
    }

    shared_ptr<Node> Node::duplicateInstance() {
        return make_shared<Node>(*this);
    }

    void Node::setVisible(const bool visible) {
        app()._lockDeferredUpdate();
        this->visible = visible;
        for (const auto &child : children) {
            child->setVisible(visible);
        }
        app()._unlockDeferredUpdate();
    }

    void Node::_onReady() {
        if (!isReady) {
            onReady();
            isReady = true;
        }
    }

}
