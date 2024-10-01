module;
#include <cassert>
#include "z0/libraries.h"

module Z0;

import :Node;
import :Application;

namespace z0 {

    bool Node::addChild(const shared_ptr<Node> child) {
        if (haveChild(child, false)) return false;
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

    bool Node::removeChild(const shared_ptr<Node>& node) {
        if (!haveChild(node, false)) return false;
        node->parent = nullptr;
        if (node->addedToScene) { Application::get()._removeNode(node); }
        children.remove(node);
        return true;
    }

    list<shared_ptr<Node>>::const_iterator Node::removeChild(const list<shared_ptr<Node>>::const_iterator& it) {
        auto& node = *it;
        node->parent = nullptr;
        if (node->addedToScene) { Application::get()._removeNode(node); }
        return children.erase(it);
    }

    void Node::removeAllChildren() {
        for(const auto& node : children) {
            node->parent = nullptr;
            if (node->addedToScene) { Application::get()._removeNode(node); }
        }
        children.clear();
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

    Application& Node::app() const {
        return Application::get();
    }
}