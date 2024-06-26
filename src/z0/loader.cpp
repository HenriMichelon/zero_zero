#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/loader.h"
#include "z0/application.h"
#endif

namespace z0 {

    // https://fastgltf.readthedocs.io/v0.7.x/tools.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    shared_ptr<Image> loadImage(fastgltf::Asset& asset, fastgltf::Image& image, VkFormat format) {
        const auto& device = Application::get()._getDevice();
        shared_ptr<Image> newImage;
        int width, height, nrChannels;
        string name{image.name};
        visit(
            fastgltf::visitor {
                [](auto& arg) {},
                [&](fastgltf::sources::URI& filePath) {
                    assert(filePath.fileByteOffset == 0); // We don't support offsets with stbi.
                    assert(filePath.uri.isLocalPath()); // We're only capable of loading
                    const string path(filePath.uri.path().begin(),
                                           filePath.uri.path().end()); // Thanks C++.
                    auto* data = stbi_load(path.c_str(), &width, &height,
                                                    &nrChannels, STBI_rgb_alpha);
                    if (data) {
                        VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                        newImage = make_shared<Image>(device, name,
                                                      width, height,
                                                      imageSize, data, format);
                        stbi_image_free(data);
                    }
                },
                [&](fastgltf::sources::Vector& vector) {
                    auto* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                                                                &width, &height,
                                                                &nrChannels, STBI_rgb_alpha);
                    if (data) {
                        VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                        newImage = make_shared<Image>(device, name,
                                                      width, height,
                                                      imageSize, data, format);
                        stbi_image_free(data);
                    }
                },
                [&](fastgltf::sources::BufferView& view) {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];

                    visit(fastgltf::visitor {
                           // We only care about VectorWithMime here, because we
                           // specify LoadExternalBuffers, meaning all buffers
                           // are already loaded into a vector.
                           [](auto& arg) {},
                           [&](fastgltf::sources::Vector& vector) {
                               auto* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height,
                                                                           &nrChannels, STBI_rgb_alpha);
                               if (data) {
                                   VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                   newImage = make_shared<Image>(device, name,
                                                                 width, height,
                                                                 imageSize, data, format);
                                   stbi_image_free(data);
                               }
                           },
                           [&](fastgltf::sources::Array& array) {
                               auto* data = stbi_load_from_memory(array.bytes.data() + bufferView.byteOffset,
                                                                           static_cast<int>(bufferView.byteLength),
                                                                           &width, &height,
                                                                           &nrChannels, STBI_rgb_alpha);
                               if (data) {
                                   VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                                   newImage = make_shared<Image>(device, name,
                                                                 width, height,
                                                                 imageSize, data, format);
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
    shared_ptr<Node> Loader::loadModelFromFile(const filesystem::path& filename, bool forceBackFaceCulling) {
        filesystem::path filepath = Application::get().getConfig().appDir / filename;
        fastgltf::Parser parser {fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_texture_transform};
        constexpr auto gltfOptions =
                fastgltf::Options::DontRequireValidAssetMember |
                fastgltf::Options::AllowDouble |
                fastgltf::Options::LoadGLBBuffers |
                fastgltf::Options::LoadExternalBuffers;
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
        for (fastgltf::Material& mat : gltf.materials) {
            shared_ptr<StandardMaterial> material = make_shared<StandardMaterial>(mat.name.data());
            if (mat.pbrData.baseColorTexture.has_value()) {
                const auto imageIndex = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
                shared_ptr<Image> image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_SRGB);
                material->setAlbedoTexture(make_shared<ImageTexture>(image));
                // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_texture_transform/README.md
                // We apply the albedo texture transform to all the textures (specular, normal, ...)
                const auto& transform = mat.pbrData.baseColorTexture.value().transform;
                if (transform != nullptr) {
                    material->setTextureTransform({ 
                        .offset = { transform->uvOffset[0], transform->uvOffset[1] },
                        .scale = { transform->uvScale[0], transform->uvScale[1] }
                    });
                }
            }
            material->setAlbedoColor(Color{
                mat.pbrData.baseColorFactor[0],
                mat.pbrData.baseColorFactor[1],
                mat.pbrData.baseColorFactor[2],
                mat.pbrData.baseColorFactor[3],
            });
            if (mat.specular != nullptr) {
                if (mat.specular->specularColorTexture.has_value()) {
                    auto imageIndex = gltf.textures[mat.specular->specularColorTexture.value().textureIndex].imageIndex.value();
                    shared_ptr<Image> image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_SRGB);
                    material->setSpecularTexture(std::make_shared<ImageTexture>(image));
                }
            }
            if (mat.normalTexture.has_value()) {
                auto imageIndex = gltf.textures[mat.normalTexture->textureIndex].imageIndex.value();
                // https://www.reddit.com/r/vulkan/comments/wksa4z/comment/jd7504e/
                shared_ptr<Image> image = loadImage(gltf, gltf.images[imageIndex], VK_FORMAT_R8G8B8A8_UNORM);
                material->setNormalTexture(std::make_shared<ImageTexture>(image));
            }
            material->setCullMode(forceBackFaceCulling ? CULLMODE_BACK : mat.doubleSided ? CULLMODE_DISABLED : CULLMODE_BACK);
            materials.push_back(material);
        }
        if (materials.empty()) {
            materials.push_back(std::make_shared<StandardMaterial>(""));
        }

        vector<shared_ptr<Mesh>> meshes;
        for (fastgltf::Mesh& glftMesh : gltf.meshes) {
            shared_ptr<Mesh> mesh = std::make_shared<Mesh>(glftMesh.name.data());
            vector<Vertex>& vertices = mesh->getVertices();
            vector<uint32_t>& indices = mesh->getIndices();
            for (auto&& p : glftMesh.primitives) {
                shared_ptr<Surface> surface = std::make_shared<Surface>(
                        static_cast<uint32_t>(indices.size()),
                        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count));
                size_t initial_vtx = vertices.size();
                bool haveTangents = false;
                // load indexes
                {
                    fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                                                             [&](std::uint32_t idx) {
                                                                 indices.push_back(idx + initial_vtx);
                                                             });
                }
                // load vertex positions
                {
                    fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                                                                  [&](vec3 v, size_t index) {
                                                                      Vertex newvtx {
                                                                          .position = v,
                                                                      };
                                                                      vertices[index + initial_vtx] = newvtx;
                                                                  });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                                                                  [&](vec3 v, size_t index) {
                                                                      vertices[index + initial_vtx].normal = v;
                                                                  });
                }
                // load UVs
                auto uv = p.findAttribute("TEXCOORD_0");
                if (uv != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                                                                  [&](vec2 v, size_t index) {
                                                                      vertices[index + initial_vtx].uv= {
                                                                          v.x,
                                                                          v.y
                                                                      };
                                                                  });
                }
                auto tangents = p.findAttribute("TANGENT");
                if (tangents != p.attributes.end()) {
                    haveTangents = true;
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*tangents).second],
                                                                  [&](vec4 v, size_t index) {
                                                                      vertices[index + initial_vtx].tangent = v;
                                                                  });
                }
                // associate material to surface and keep track of all materials used in the Mesh
                if (p.materialIndex.has_value()) {
                    auto material = materials[p.materialIndex.value()];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);

                }
                // calculate tangent for each triangle
                if (!haveTangents) {
                    for (uint32_t i = 0; i < indices.size(); i += 3) {
                        auto &vertex1 = vertices[indices[i]];
                        auto &vertex2 = vertices[indices[i + 1]];
                        auto &vertex3 = vertices[indices[i + 2]];
                        vec3 edge1 = vertex2.position - vertex1.position;
                        vec3 edge2 = vertex3.position - vertex1.position;
                        vec2 deltaUV1 = vertex2.uv - vertex1.uv;
                        vec2 deltaUV2 = vertex3.uv - vertex1.uv;

                        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                        vec3 tangent {
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
            meshes.push_back(mesh);
        }

        // load all nodes and their meshes
        vector<shared_ptr<Node>> nodes;
        for (fastgltf::Node& node : gltf.nodes) {
            shared_ptr<Node> newNode;
            string name{node.name.data()};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the MeshInstance class
            if (node.meshIndex.has_value()) {
                auto mesh = meshes[*node.meshIndex];
                mesh->_buildModel();
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }

            visit(fastgltf::visitor { [&](fastgltf::Node::TransformMatrix matrix) {
                           memcpy(&newNode->_getTransformLocal(), matrix.data(), sizeof(matrix));
                       },
                                           [&](fastgltf::TRS transform) {
                                               vec3 tl(transform.translation[0], transform.translation[1],
                                                            transform.translation[2]);
                                               quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1],
                                                             transform.rotation[2]);
                                               vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                                               mat4 tm = translate(mat4(1.f), tl);
                                               mat4 rm = toMat4(rot);
                                               mat4 sm = scale(mat4(1.f), sc);

                                               newNode->_setTransform(tm * rm * sm);
                                           } },
                       node.transform);
            newNode->_updateTransform(mat4{1.0f});
            nodes.push_back(newNode);
        }

        // Build node tree
        for (uint32_t i = 0; i < gltf.nodes.size(); i++) {
            fastgltf::Node& node = gltf.nodes[i];
            shared_ptr<Node>& sceneNode = nodes[i];
            for (auto& child : node.children) {
                sceneNode->setProcessMode(PROCESS_MODE_DISABLED);
                sceneNode->addChild(nodes[child]);
            }
        }

        // find the top nodes, with no parents
        shared_ptr<Node> rootNode = make_shared<Node>(filename.string());
        for (auto& node : nodes) {
            if (node->getParent() == nullptr) {
                node->setProcessMode(PROCESS_MODE_DISABLED);
                rootNode->addChild(node);
            }
        }

        return rootNode;
    }
    
    void Loader::addNode(shared_ptr<Node>& parent, 
                         map<string, shared_ptr<Node>>& nodeTree, 
                         map<string, SceneNode>& sceneTree, 
                         const Loader::SceneNode& nodeDesc) {
        if (nodeTree.contains(nodeDesc.id)) {
            die("Node with id", nodeDesc.id, "already exists in the scene tree");
        }
        sceneTree[nodeDesc.id] = nodeDesc;
        shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "model") {
                node = Loader::loadModelFromFile(nodeDesc.resource);
            }
        } else {
            if (nodeDesc.clazz == "") {
                node = make_shared<Node>();
            } else {
                node = TypeRegistry::makeShared<Node>(nodeDesc.clazz);
            }
            for(const auto& prop: nodeDesc.properties) {
                if (prop.first == "position") {
                    node->setPosition(to_vec3(prop.second));
                } else if (prop.first == "rotation") {
                    auto rot = to_vec3(prop.second);
                    node->setRotation(vec3{radians(rot.x), radians(rot.y), radians(rot.z)});
                }
            }
            for(const auto& child: nodeDesc.children) {
                if (nodeTree.contains(child.id)) {
                    if (sceneTree[child.id].isResource) {
                        node->addChild(nodeTree[child.id]->duplicate());
                    } else {
                        node->addChild(nodeTree[child.id]);
                    }
                } else {
                    addNode(parent, nodeTree, sceneTree, child);
                }
            }
            parent->addChild(node);
        }
        nodeTree[nodeDesc.id] = node;
    }

    void Loader::addSceneFromFile(shared_ptr<Node>& parent, const filesystem::path& filename) {
        filesystem::path filepath = Application::get().getConfig().appDir / filename;
        map<string, shared_ptr<Node>> nodeTree;
        map<string, SceneNode> sceneTree;
        for(const auto& nodeDesc: loadSceneFromJSON(filepath.string())) {
            addNode(parent, nodeTree, sceneTree, nodeDesc);
        }
    }

    void from_json(const nlohmann::json& j, Loader::SceneNode& node) {
        j.at("id").get_to(node.id);
        node.isResource = j.contains("resource");
        if (node.isResource) {
            j.at("resource").get_to(node.resource);
            j.at("type").get_to(node.resourceType);
        } else {
            if (j.contains("class")) j.at("class").get_to(node.clazz);
            if (j.contains("properties")) j.at("properties").get_to(node.properties);
            if (j.contains("children")) j.at("children").get_to(node.children);
        }
    }

    vector<Loader::SceneNode> Loader::loadSceneFromJSON(const string& filepath) {
        ifstream ifs(filepath);
        vector<Loader::SceneNode> nodes;
        try {
            auto jsonData = nlohmann::json::parse(ifs);
            nodes = jsonData["nodes"];
        } catch(nlohmann::json::parse_error) {
            die("Error loading", filepath);
        }
        return nodes;
    }

}