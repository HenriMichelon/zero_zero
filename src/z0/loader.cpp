module;
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <json.hpp>
#include <volk.h>
#include "stb_image.h"
#include "z0/libraries.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

module z0;

import :Tools;
import :Constants;
import :Application;
import :Image;
import :Material;
import :Texture;
import :Color;
import :Mesh;
import :Node;
import :MeshInstance;
import :TypeRegistry;
import :Loader;

namespace z0 {

    // https://fastgltf.readthedocs.io/v0.7.x/tools.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    shared_ptr<Image> loadImage(fastgltf::Asset &asset, fastgltf::Image &image, VkFormat format) {
        const auto       &device = Application::get()._getDevice();
        shared_ptr<Image> newImage;
        int               width, height, nrChannels;
        string            name{image.name};
        visit(fastgltf::visitor{
                      [](auto &arg) {},
                      [&](fastgltf::sources::URI &filePath) {
                          assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                          assert(filePath.uri.isLocalPath()); // We're only capable of loading
                          const string path(filePath.uri.path().begin(),
                                            filePath.uri.path().end()); // Thanks C++.
                          auto        *data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
                          if (data) {
                              VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                              newImage = make_shared<Image>(device, name, width, height, imageSize, data, format);
                              stbi_image_free(data);
                          }
                      },
                      [&](fastgltf::sources::Vector &vector) {
                          auto *data = stbi_load_from_memory(vector.bytes.data(),
                                                             static_cast<int>(vector.bytes.size()),
                                                             &width,
                                                             &height,
                                                             &nrChannels,
                                                             STBI_rgb_alpha);
                          if (data) {
                              VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                              newImage = make_shared<Image>(device, name, width, height, imageSize, data, format);
                              stbi_image_free(data);
                          }
                      },
                      [&](fastgltf::sources::BufferView &view) {
                          auto &bufferView = asset.bufferViews[view.bufferViewIndex];
                          auto &buffer     = asset.buffers[bufferView.bufferIndex];

                          visit(fastgltf::visitor{
                                        // We only care about VectorWithMime here, because we
                                        // specify LoadExternalBuffers, meaning all buffers
                                        // are already loaded into a vector.
                                        [](auto &arg) {},
                                        [&](fastgltf::sources::Vector &vector) {
                                            auto *data =
                                                    stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                          static_cast<int>(bufferView.byteLength),
                                                                          &width,
                                                                          &height,
                                                                          &nrChannels,
                                                                          STBI_rgb_alpha);
                                            if (data) {
                                                VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                                newImage               = make_shared<Image>(
                                                        device, name, width, height, imageSize, data, format);
                                                stbi_image_free(data);
                                            }
                                        },
                                        [&](fastgltf::sources::Array &array) {
                                            auto *data =
                                                    stbi_load_from_memory(array.bytes.data() + bufferView.byteOffset,
                                                                          static_cast<int>(bufferView.byteLength),
                                                                          &width,
                                                                          &height,
                                                                          &nrChannels,
                                                                          STBI_rgb_alpha);
                                            if (data) {
                                                VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                                newImage               = make_shared<Image>(
                                                        device, name, width, height, imageSize, data, format);
                                                stbi_image_free(data);
                                            }
                                        },
                                },
                                buffer.data);
                      },
              },
              image.data);
        return newImage;
    }

    // https://fastgltf.readthedocs.io/v0.7.x/overview.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    shared_ptr<Node> Loader::loadModelFromFile(const filesystem::path &filename, bool forceBackFaceCulling,
                                               bool commandLineMode) {
        filesystem::path filepath = commandLineMode ? filename : (Application::get().getConfig().appDir / filename);
        fastgltf::Parser parser{fastgltf::Extensions::KHR_materials_specular |
                                fastgltf::Extensions::KHR_texture_transform};
        constexpr auto   gltfOptions = fastgltf::Options::DontRequireValidAssetMember | fastgltf::Options::AllowDouble |
                fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;
        fastgltf::GltfDataBuffer data;
        if (!data.loadFromFile(filepath)) {
            die("Error loading", filename.string());
        }
        auto asset = parser.loadGltfBinary(&data, filepath.parent_path(), gltfOptions);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            die(getErrorMessage(error));
        }
        fastgltf::Asset gltf = std::move(asset.get());

        // load all materials
        vector<std::shared_ptr<StandardMaterial>> materials{};
        for (fastgltf::Material &mat : gltf.materials) {
            auto material = make_shared<StandardMaterial>(mat.name.data());
            if (!commandLineMode && mat.pbrData.baseColorTexture.has_value()) {
                const auto imageIndex =
                        gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
                auto image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_SRGB);
                material->setAlbedoTexture(make_shared<ImageTexture>(image));
                // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md
                // We apply the albedo texture transform to all the textures (specular, normal, ...)
                const auto &transform = mat.pbrData.baseColorTexture.value().transform;
                if (transform != nullptr) {
                    material->setTextureTransform({.offset = {transform->uvOffset[0], transform->uvOffset[1]},
                                                   .scale  = {transform->uvScale[0], transform->uvScale[1]}});
                }
            }
            material->setAlbedoColor(Color{
                    mat.pbrData.baseColorFactor[0],
                    mat.pbrData.baseColorFactor[1],
                    mat.pbrData.baseColorFactor[2],
                    mat.pbrData.baseColorFactor[3],
            });
            switch (mat.alphaMode) {
            case fastgltf::AlphaMode::Blend:
                material->setTransparency(TRANSPARENCY_ALPHA);
                material->setAlphaScissor(mat.alphaCutoff);
                break;
            case fastgltf::AlphaMode::Mask:
                material->setTransparency(TRANSPARENCY_SCISSOR);
                break;
            default:
                break;
            }
            if (mat.specular != nullptr) {
                if (!commandLineMode && mat.specular->specularColorTexture.has_value()) {
                    auto imageIndex =
                            gltf.textures[mat.specular->specularColorTexture.value().textureIndex].imageIndex.value();
                    auto image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_SRGB);
                    material->setSpecularTexture(std::make_shared<ImageTexture>(image));
                }
            }
            if (!commandLineMode && mat.normalTexture.has_value()) {
                auto imageIndex = gltf.textures[mat.normalTexture->textureIndex].imageIndex.value();
                // https://www.reddit.com/r/vulkan/comments/wksa4z/comment/jd7504e/
                auto image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_UNORM);
                material->setNormalTexture(std::make_shared<ImageTexture>(image));
            }
            material->setCullMode(forceBackFaceCulling      ? CULLMODE_BACK
                                          : mat.doubleSided ? CULLMODE_DISABLED
                                                            : CULLMODE_BACK);
            materials.push_back(material);
        }
        log("Loader :", to_string(materials.size()), "materials");
        if (materials.empty()) {
            materials.push_back(std::make_shared<StandardMaterial>(""));
        }

        auto meshes = vector<shared_ptr<Mesh>>(gltf.meshes.size());
        auto meshesCount = 0;
        for (size_t i = 0; i < gltf.meshes.size(); i++) {
        // for (fastgltf::Mesh &glftMesh : gltf.meshes) {
            const auto& glftMesh = gltf.meshes[i];
            auto  mesh     = std::make_shared<Mesh>(glftMesh.name.data());
            auto &vertices = mesh->getVertices();
            auto &indices  = mesh->getIndices();
            for (auto &&p : glftMesh.primitives) {
                auto surface = std::make_shared<Surface>(
                        static_cast<uint32_t>(indices.size()),
                        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count));
                auto initial_vtx  = vertices.size();
                auto   haveTangents = false;
                // load indexes
                {
                    auto &indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(
                            gltf, indexaccessor, [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });
                }
                // load vertex positions
                {
                    auto &posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](vec3 v, size_t index) {
                        Vertex newvtx{
                                .position = v,
                        };
                        vertices[index + initial_vtx] = newvtx;
                    });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(
                            gltf, gltf.accessors[(*normals).second], [&](vec3 v, size_t index) {
                                vertices[index + initial_vtx].normal = v;
                            });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(
                            gltf, gltf.accessors[(*uv).second], [&](vec2 v, size_t index) {
                                vertices[index + initial_vtx].uv = {v.x, v.y};
                            });
                }
                auto tangents = p.findAttribute("TANGENT");
                if (tangents != p.attributes.end()) {
                    haveTangents = true;
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(
                            gltf, gltf.accessors[(*tangents).second], [&](vec4 v, size_t index) {
                                vertices[index + initial_vtx].tangent = v;
                            });
                }
                // associate material to surface and keep track of all materials used in the Mesh
                if (p.materialIndex.has_value()) {
                    auto material     = materials[p.materialIndex.value()];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                }
                // calculate tangent for each triangle
                if (!haveTangents) {
                    for (uint32_t i = 0; i < indices.size(); i += 3) {
                        auto &vertex1  = vertices[indices[i]];
                        auto &vertex2  = vertices[indices[i + 1]];
                        auto &vertex3  = vertices[indices[i + 2]];
                        vec3  edge1    = vertex2.position - vertex1.position;
                        vec3  edge2    = vertex3.position - vertex1.position;
                        vec2  deltaUV1 = vertex2.uv - vertex1.uv;
                        vec2  deltaUV2 = vertex3.uv - vertex1.uv;

                        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                        vec3  tangent{
                                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z),
                        };
                        vertex1.tangent = vec4(tangent, 1.0);
                        vertex2.tangent = vec4(tangent, 1.0);
                        vertex3.tangent = vec4(tangent, 1.0);
                    }
                }
                mesh->getSurfaces().push_back(surface);
            }
            // Check if we already have a similar mesh
            auto it = std::find(meshes.begin(), meshes.end(), mesh);
            if (it != meshes.end()) {
                // Don't use duplicate meshes
                meshes[i] = *it;
            } else {
                meshes[i] = mesh;
                meshesCount++;
            }
        }
        log("Loader :", to_string(meshes.size()), "meshes", to_string(meshesCount), "uniques meshes");

        // load all nodes and their meshes
        vector<shared_ptr<Node>> nodes;
        for (fastgltf::Node &node : gltf.nodes) {
            shared_ptr<Node> newNode;
            string           name{node.name.data()};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the
            // MeshInstance class
            if (node.meshIndex.has_value()) {
                auto mesh = meshes[*node.meshIndex];
                if (!commandLineMode)
                    mesh->_buildModel();
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }

            visit(fastgltf::visitor{
                          [&](fastgltf::Node::TransformMatrix matrix) {
                              memcpy(&newNode->_getTransformLocal(), matrix.data(), sizeof(matrix));
                          },
                          [&](fastgltf::TRS transform) {
                              vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
                              quat rot(transform.rotation[3],
                                       transform.rotation[0],
                                       transform.rotation[1],
                                       transform.rotation[2]);
                              vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                              mat4 tm = translate(mat4(1.f), tl);
                              mat4 rm = toMat4(rot);
                              mat4 sm = scale(mat4(1.f), sc);

                              newNode->_setTransform(tm * rm * sm);
                          }},
                  node.transform);
            newNode->_updateTransform(mat4{1.0f});
            nodes.push_back(newNode);
        }

        // Build node tree
        for (uint32_t i = 0; i < gltf.nodes.size(); i++) {
            fastgltf::Node   &node      = gltf.nodes[i];
            shared_ptr<Node> &sceneNode = nodes[i];
            for (auto &child : node.children) {
                sceneNode->setProcessMode(PROCESS_MODE_DISABLED);
                sceneNode->addChild(nodes[child]);
            }
        }

        // find the top nodes, with no parents
        shared_ptr<Node> rootNode = make_shared<Node>(filename.string());
        for (auto &node : nodes) {
            if (node->getParent() == nullptr) {
                node->setProcessMode(PROCESS_MODE_DISABLED);
                rootNode->addChild(node);
            }
        }

        return rootNode;
    }

    void Loader::addNode(Node *parent, map<string, shared_ptr<Node>> &nodeTree, map<string, SceneNode> &sceneTree,
                         const SceneNode &nodeDesc, const bool editorMode) {
        constexpr auto log_name{"Scene loader :"};
        if (nodeTree.contains(nodeDesc.id))
            die(log_name, "Node with id", nodeDesc.id, "already exists in the scene tree");
        sceneTree[nodeDesc.id] = nodeDesc;
        shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "model") {
                // the model is in a glTF file
                node = Loader::loadModelFromFile(nodeDesc.resource);
                node->setName(nodeDesc.id);
            } else if (nodeDesc.resourceType == "mesh") {
                // the model is part of another, already loaded, model
                if (nodeTree.contains(nodeDesc.resource)) {
                    // get the parent resource
                    auto &resource = nodeTree[nodeDesc.resource];
                    // get the mesh node via the relative path
                    node = resource->getNode(nodeDesc.resourcePath);
                    if (node == nullptr)
                        die(log_name, "Mesh with path", nodeDesc.resourcePath, "not found");
                } else {
                    die(log_name, "Resource with id", nodeDesc.resource, "not found");
                }
            }
        } else {
            if ((nodeDesc.clazz.empty()) || (nodeDesc.isCustom)) {
                // The node class is a game class
                node = make_shared<Node>(nodeDesc.id);
            } else {
                // The node class is a engine registered class
                node = TypeRegistry::makeShared<Node>(nodeDesc.clazz);
                node->setName(nodeDesc.id);
            }
            node->_setParent(parent);
            if (nodeDesc.child != nullptr) {
                // If we have a designated child we mimic the position, rotation and scale of the child
                auto &child = nodeTree[nodeDesc.child->id];
                if (child == nullptr)
                    die(log_name, "Child node", nodeDesc.child->id, "not found");
                node->setPositionGlobal(child->getPositionGlobal());
                node->setRotation(child->getRotation());
                node->setScale(child->getScale());
                if (nodeDesc.child->needDuplicate) {
                    child = child->duplicate();
                }
                child->setPosition(VEC3ZERO);
                child->setRotation(QUATERNION_IDENTITY);
                child->setScale(1.0f);
                if (child->getParent() != nullptr) {
                    child->getParent()->removeChild(child);
                };
                node->addChild(child);
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
                    addNode(node.get(), nodeTree, sceneTree, child, editorMode);
                }
            }
            for (const auto &prop : nodeDesc.properties) {
                node->setProperty(to_lower(prop.first), prop.second);
            }
            node->_setParent(nullptr);
            parent->addChild(node);
            if (editorMode) {
                node->setProcessMode(PROCESS_MODE_DISABLED);
            }
        }
        nodeTree[nodeDesc.id] = node;
    }

    void Loader::addSceneFromFile(shared_ptr<Node> &parent, const filesystem::path &filename, const bool editorMode) {
        addSceneFromFile(parent.get(), filename, editorMode);
    }

    void Loader::addSceneFromFile(Node *parent, const filesystem::path &filename, const bool editorMode) {
        filesystem::path              filepath = Application::get().getConfig().appDir / filename;
        map<string, shared_ptr<Node>> nodeTree;
        map<string, SceneNode>        sceneTree;
        for (const auto &nodeDesc : loadSceneFromJSON(filepath)) {
            addNode(parent, nodeTree, sceneTree, nodeDesc, editorMode);
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

    vector<Loader::SceneNode> Loader::loadSceneFromJSON(const std::filesystem::path &filepath) {
        ifstream          ifs(filepath);
        vector<SceneNode> scene{};
        try {
            auto jsonData = nlohmann::ordered_json::parse(ifs); // parsing using ordered_json to preserver fields order
            if (jsonData.contains("includes")) {
                vector<string> includes = jsonData["includes"];
                for (const auto &include : includes) {
                    vector<SceneNode> includeNodes = loadSceneFromJSON(filepath.parent_path() / include);
                    scene.append_range(includeNodes);
                }
            }
            vector<SceneNode> nodes = jsonData["nodes"];
            scene.append_range(nodes);
        } catch (nlohmann::json::parse_error) {
            die("Error loading scene from JSON file ", filepath.string());
        }
        return scene;
    }
} // namespace z0
