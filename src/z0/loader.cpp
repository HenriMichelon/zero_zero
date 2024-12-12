/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <json.hpp>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.Loader;

import z0.Application;
import z0.Constants;
import z0.GlTF;
import z0.Tools;
import z0.TypeRegistry;
import z0.VirtualFS;
import z0.ZRes;

import z0.nodes.AnimationPlayer;
import z0.nodes.Node;

namespace z0 {

    mutex Loader::resourcesMutex;

    void Loader::load(const shared_ptr<Node>&rootNode, const string& filepath, const bool usecache) {
        if (filepath.ends_with(".json")) {
            return loadScene(rootNode, filepath);
        }
        if (filepath.ends_with(".zres")) {
            ZRes::load(rootNode, filepath);
        }
        else if (filepath.ends_with(".gltf") || filepath.ends_with(".glb")) {
            GlTF::load(rootNode, filepath);
        } else {
            die("Loader : unsupported file format for", filepath);
        }
        if (usecache) {
            auto lock = lock_guard(resourcesMutex);
            resources[filepath] = rootNode;
        }
    }

    void Loader::_cleanup() {
        resources.clear();
    }

    void Loader::addNode(Node *parent,
                         map<string, shared_ptr<Node>> &nodeTree,
                         map<string, SceneNode> &sceneTree,
                         const SceneNode &nodeDesc) {
        constexpr auto log_name{"Scene loader :"};
        if (nodeTree.contains(nodeDesc.id)) {
            die(log_name, "Node with id", nodeDesc.id, "already exists in the scene tree");
        }
        sceneTree[nodeDesc.id] = nodeDesc;
        shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "resource") {
                // the model is in a glTF/ZScene file
                node = load(nodeDesc.resource, true);
                node->setName(nodeDesc.id);
            } else if (nodeDesc.resourceType == "mesh") {
                // the model is part of another, already loaded, model
                if (nodeTree.contains(nodeDesc.resource)) {
                    // get the parent resource
                    const auto &resource = nodeTree[nodeDesc.resource];
                    // get the mesh node via the relative path
                    node = resource->getChildByPath(nodeDesc.resourcePath);
                    if (node == nullptr) {
                        resource->printTree();
                        die(log_name, "Mesh with path", nodeDesc.resourcePath, "not found");
                    }
                } else {
                    die(log_name, "Resource with id", nodeDesc.resource, "not found");
                }
            }
        } else {
            if (nodeDesc.clazz.empty() || nodeDesc.isCustom) {
                node = make_shared<Node>(nodeDesc.id);
            } else {
                // The node class is a registered class
                node = TypeRegistry::makeShared<Node>(nodeDesc.clazz);
                node->setName(nodeDesc.id);
            }
            node->_setParent(parent);
            auto parentNode = node;
            auto childrenList = nodeDesc.children;
            if (nodeDesc.child != nullptr) {
                auto child = nodeTree[nodeDesc.child->id];
                if (child == nullptr) {
                    die(log_name, "Child node", nodeDesc.child->id, "not found");
                }
                if (nodeDesc.child->needDuplicate) {
                    child = child->duplicate();
                }
                child->setPosition(VEC3ZERO);
                child->setRotation(QUATERNION_IDENTITY);
                child->setScale(1.0f);
                if (child->getParent() != nullptr) {
                    child->getParent()->removeChild(child);
                }
                node->addChild(child);
                parentNode = child;
                childrenList = nodeDesc.child->children;
            }
            for (const auto &child : childrenList) {
                if (nodeTree.contains(child.id)) {
                    auto &childNode = nodeTree[child.id];
                    if (child.needDuplicate) {
                        parentNode->addChild(childNode->duplicate());
                    } else {
                        if (childNode->getParent() != nullptr) {
                            childNode->getParent()->removeChild(childNode);
                        }
                        parentNode->addChild(childNode);
                    }
                } else {
                    addNode(parentNode.get(), nodeTree, sceneTree, child);
                }
            }
            for (const auto &prop : nodeDesc.properties) {
                node->setProperty(prop.first, prop.second);
            }

            node->_setParent(nullptr);
            if (!nodeDesc.isIncluded) {
                parent->addChild(node);
            }
        }
        // cout << node->getName() << " -> " << nodeDesc.id << endl;
        nodeTree[nodeDesc.id] = node;
    }

    void Loader::loadScene(const shared_ptr<Node>&rootNode, const string &filepath) {
        // const auto tStart = chrono::high_resolution_clock::now();
        map<string, shared_ptr<Node>> nodeTree;
        map<string, SceneNode>        sceneTree;
        for (const auto &nodeDesc : loadSceneDescriptionFromJSON(filepath)) {
            addNode(rootNode.get(), nodeTree, sceneTree, nodeDesc);
            // log("addNode", nodeDesc.id);
        }
        // https://jrouwe.github.io/JoltPhysics/class_physics_system.html#ab3cd9f2562f0f051c032b3bc298d9604
        app()._getPhysicsSystem().OptimizeBroadPhase();
        // const auto last_time = chrono::duration<float, milli>(chrono::high_resolution_clock::now() - tStart).count();
        // log("loadScene loading time ", to_string(last_time));
    }

    void from_json(const nlohmann::ordered_json &j, Loader::SceneNode &node) {
        j.at("id").get_to(node.id);
        node.isResource    = j.contains("resource");
        node.isCustom      = j.contains("custom");
        node.needDuplicate = j.contains("duplicate");
        if (node.isResource) {
            j.at("resource").get_to(node.resource);
            j.at("type").get_to(node.resourceType);
            if (j.contains("path"))
                j.at("path").get_to(node.resourcePath);
        } else {
            if (j.contains("class"))
                j.at("class").get_to(node.clazz);
            if (j.contains("properties")) {
                for (auto &[k, v] : j.at("properties").items()) {
                    node.properties.push_back({k, v});
                }
            }
            if (j.contains("child")) {
                node.child = make_shared<Loader::SceneNode>();
                j.at("child").get_to(*(node.child));
            }
            if (j.contains("children"))
                j.at("children").get_to(node.children);
        }
    }

    vector<Loader::SceneNode> Loader::loadSceneDescriptionFromJSON(const string &filepath) {
        vector<SceneNode> scene{};
        try {
            auto jsonData = nlohmann::ordered_json::parse(VirtualFS::openReadStream(filepath)); // parsing using ordered_json to preserver fields order
            if (jsonData.contains("includes")) {
                const vector<string> includes = jsonData["includes"];
                for (const auto &include : includes) {
                    vector<SceneNode> includeNodes = loadSceneDescriptionFromJSON(include);
                    for(auto& node : includeNodes) {
                        node.isIncluded = true;
                    }
                    scene.append_range(includeNodes);
                }
            }
            vector<SceneNode> nodes = jsonData["nodes"];
            scene.append_range(nodes);
        } catch (nlohmann::json::parse_error) {
            die("Error loading scene from JSON file ", filepath);
        }
        return scene;
    }
} // namespace z0
