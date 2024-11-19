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

import z0.Constants;
import z0.GlTF;
import z0.Node;
import z0.Tools;
import z0.TypeRegistry;
import z0.VirtualFS;
import z0.ZRes;

namespace z0 {

    shared_ptr<Node> Loader::load(const string& filepath) {
        if (filepath.ends_with(".zres")) {
            return ZRes::load(filepath);
        }
        if (filepath.ends_with(".gltf") || filepath.ends_with(".glb")) {
            return GlTF::load(filepath);
        }
        die("Loader : unsupported scene file format");
        return nullptr;
    }

    void Loader::addNode(Node *parent, map<string, shared_ptr<Node>> &nodeTree, map<string, SceneNode> &sceneTree,
                         const SceneNode &nodeDesc) {
        constexpr auto log_name{"Scene loader :"};
        if (nodeTree.contains(nodeDesc.id)) {
            die(log_name, "Node with id", nodeDesc.id, "already exists in the scene tree");
        }
        sceneTree[nodeDesc.id] = nodeDesc;
        shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "model") {
                // the model is in a glTF/ZScene file
                node = load(nodeDesc.resource);
                node->setName(nodeDesc.id);
            } else if (nodeDesc.resourceType == "mesh") {
                // the model is part of another, already loaded, model
                if (nodeTree.contains(nodeDesc.resource)) {
                    // get the parent resource
                    const auto &resource = nodeTree[nodeDesc.resource];
                    // get the mesh node via the relative path
                    node = resource->getNode(nodeDesc.resourcePath);
                    if (node == nullptr) {
                        resource->printTree();
                        die(log_name, "Mesh with path", nodeDesc.resourcePath, "not found");
                    }
                } else {
                    die(log_name, "Resource with id", nodeDesc.resource, "not found");
                }
            }
        } else {
            if ((nodeDesc.clazz.empty()) || (nodeDesc.isCustom)) {
                node = make_shared<Node>(nodeDesc.id);
            } else {
                // The node class is an engine registered class
                node = TypeRegistry::makeShared<Node>(nodeDesc.clazz);
                node->setName(nodeDesc.id);
            }
            node->_setParent(parent);
            if (nodeDesc.child != nullptr) {
                // If we have a designated child we mimic the position, rotation and scale of the child
                const auto& child = nodeTree[nodeDesc.child->id];
                if (child == nullptr)
                    die(log_name, "Child node", nodeDesc.child->id, "not found");
                node->setPositionGlobal(child->getPositionGlobal());
                node->setRotation(child->getRotation());
                node->setScale(child->getScale());
                if (nodeDesc.child->needDuplicate) {
                    const auto dup = child->duplicate();
                    dup->setPosition(VEC3ZERO);
                    dup->setRotation(QUATERNION_IDENTITY);
                    dup->setScale(1.0f);
                    if (dup->getParent() != nullptr) {
                        dup->getParent()->removeChild(dup);
                    };
                    node->addChild(dup);
                } else {
                    child->setPosition(VEC3ZERO);
                    child->setRotation(QUATERNION_IDENTITY);
                    child->setScale(1.0f);
                    if (child->getParent() != nullptr) {
                        child->getParent()->removeChild(child);
                    };
                    node->addChild(child);
                }
            }
            for (const auto &child : nodeDesc.children) {
                if (nodeTree.contains(child.id)) {
                    auto &childNode = nodeTree[child.id];
                    if (child.needDuplicate) {
                        node->addChild(childNode->duplicate());
                    } else {
                        if (childNode->getParent() != nullptr) {
                            childNode->getParent()->removeChild(childNode);
                        };
                        node->addChild(childNode);
                    }
                } else {
                    addNode(node.get(), nodeTree, sceneTree, child);
                }
            }
            for (const auto &prop : nodeDesc.properties) {
                node->setProperty(to_lower(prop.first), prop.second);
            }
            node->_setParent(nullptr);
            if (!nodeDesc.isIncluded) parent->addChild(node);
        }
        nodeTree[nodeDesc.id] = node;
    }

    void Loader::addScene(const shared_ptr<Node> &parent, const string &filepath) {
        addScene(parent.get(), filepath);
    }

    void Loader::addScene(Node *parent, const string &filepath) {
        map<string, shared_ptr<Node>> nodeTree;
        map<string, SceneNode>        sceneTree;
        for (const auto &nodeDesc : loadSceneDescriptionFromJSON(filepath)) {
            addNode(parent, nodeTree, sceneTree, nodeDesc);
            // log("addNode", nodeDesc.id);
        }
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
            auto jsonData = nlohmann::ordered_json::parse(VirtualFS::openStream(filepath)); // parsing using ordered_json to preserver fields order
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
