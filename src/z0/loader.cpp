/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <json.hpp>
#include <volk.h>
#include <stb_image.h>
#include <ktx.h>
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
import :Node;
import :Resource;
import :MeshInstance;
import :TypeRegistry;
import :Loader;
import :VirtualFS;

import :Device;
import :VulkanImage;
import :VulkanMesh;

namespace z0 {

    typedef  std::function<shared_ptr<Image>(
                            const string&  name,
                            const void   * srcData,
                            size_t   size,
                            VkFormat format)> LoadImageFunction;

    // https://fastgltf.readthedocs.io/v0.7.x/tools.html
    // https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters-1.3-wip/chapter-5/vk_loader.cpp
    shared_ptr<Image> loadImage(
        fastgltf::Asset &asset,
        fastgltf::Image &image,
        const VkFormat format,
        const LoadImageFunction& loadImageFunction) {
        shared_ptr<Image> newImage;
        string            name{image.name};
        // log("Load image ", name);
        visit(fastgltf::visitor{
              [](auto &arg) {},
              [&](fastgltf::sources::URI &filePath) {
                  die("External textures files for glTF not supported");
              },
              [&](fastgltf::sources::Vector &vector) {
                  newImage = loadImageFunction(name, vector.bytes.data(), vector.bytes.size(), format);
              },
              [&](fastgltf::sources::BufferView &view) {
                  const auto &bufferView = asset.bufferViews[view.bufferViewIndex];
                  auto &buffer           = asset.buffers[bufferView.bufferIndex];
                  visit(fastgltf::visitor{
                        // We only care about VectorWithMime here, because we
                        // specify LoadExternalBuffers, meaning all buffers
                        // are already loaded into a vector.
                        [](auto &arg) {},
                        [&](fastgltf::sources::Vector &vector) {
                            newImage = loadImageFunction(name, vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength, format);
                        },
                        [&](fastgltf::sources::Array &array) {
                            newImage = loadImageFunction(name, array.bytes.data() + bufferView.byteOffset, bufferView.byteLength, format);
                        },
                    },
                    buffer.data);
              },
              },
              image.data);
        return newImage;
    }

    shared_ptr<Node> Loader::loadModelFromFile(const string &filepath,
                                               bool forceBackFaceCulling) {
        const auto &device = Device::get();
        auto getter = VirtualFS::openGltf(filepath);
        fastgltf::Parser parser{fastgltf::Extensions::KHR_materials_specular |
                                fastgltf::Extensions::KHR_texture_transform |
                                fastgltf::Extensions::KHR_texture_basisu |
                                fastgltf::Extensions::KHR_materials_emissive_strength};
        constexpr auto gltfOptions =
            fastgltf::Options::DontRequireValidAssetMember |
            fastgltf::Options::AllowDouble |
            fastgltf::Options::LoadGLBBuffers |
            fastgltf::Options::LoadExternalBuffers;
        auto asset = parser.loadGltfBinary(*getter, ".", gltfOptions);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            die(getErrorMessage(error));
        }
        fastgltf::Asset gltf = std::move(asset.get());

        // Already loaded images
        map<size_t, shared_ptr<Image>> images;
        map<size_t, shared_ptr<Image>> compressedImages;

        // load all materials
        vector<std::shared_ptr<StandardMaterial>> materials{};
        map<Resource::id_t, int> materialsTextCoords;
        for (fastgltf::Material &mat : gltf.materials) {
            auto material = make_shared<StandardMaterial>(mat.name.data());
            material->setAlbedoColor(Color{
                    mat.pbrData.baseColorFactor[0],
                    mat.pbrData.baseColorFactor[1],
                    mat.pbrData.baseColorFactor[2],
                    mat.pbrData.baseColorFactor[3],
            });
            material->setMetallicFactor(mat.pbrData.metallicFactor);
            material->setRoughnessFactor(mat.pbrData.roughnessFactor);
            material->setEmissiveFactor(vec3{mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]});
            material->setEmissiveStrength(mat.emissiveStrength);
            switch (mat.alphaMode) {
            case fastgltf::AlphaMode::Blend:
                material->setTransparency(TRANSPARENCY_ALPHA);
                break;
            case fastgltf::AlphaMode::Mask:
                material->setTransparency(TRANSPARENCY_SCISSOR);
                material->setAlphaScissor(mat.alphaCutoff);
                break;
            default:
                break;
            }
            auto magFilter = VK_FILTER_LINEAR;
            auto minFilter = VK_FILTER_LINEAR;
            auto wrapU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            auto wrapV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            auto convertSamplerData = [&](const fastgltf::Texture &texture) {
                const auto& sampler = gltf.samplers[texture.samplerIndex.value()];
                if (sampler.magFilter.has_value())
                    magFilter = sampler.magFilter.value() == fastgltf::Filter::Linear ? VK_FILTER_LINEAR : VK_FILTER_LINEAR;
                if (sampler.minFilter.has_value()) {
                    const auto v = sampler.minFilter.value();
                    minFilter = (v==fastgltf::Filter::Linear || v==fastgltf::Filter::LinearMipMapLinear || v==fastgltf::Filter::LinearMipMapNearest) ?
                        VK_FILTER_LINEAR : VK_FILTER_LINEAR;
                }
                wrapU = sampler.wrapS ==
                    fastgltf::Wrap::ClampToEdge ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE :
                        sampler.wrapS == fastgltf::Wrap::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT :
                            VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
                wrapV = sampler.wrapT ==
                    fastgltf::Wrap::ClampToEdge ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE :
                        sampler.wrapT == fastgltf::Wrap::Repeat ? VK_SAMPLER_ADDRESS_MODE_REPEAT :
                            VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            };
            auto loadImageRGBA = [&](const string&  name,
                                     const void   * srcData,
                                     const size_t   size,
                                     const VkFormat format) -> shared_ptr<Image> {
                int width, height, channels;
                auto *data = stbi_load_from_memory(static_cast<stbi_uc const *>(srcData),
                                                            static_cast<int>(size),
                                                            &width,
                                                            &height,
                                                            &channels,
                                                            STBI_rgb_alpha);
                if (data) {
                    const VkDeviceSize imageSize = width * height * STBI_rgb_alpha;
                    const auto newImage = make_shared<VulkanImage>(
                        device, name,
                        width, height,
                        imageSize, data, format,
                          magFilter, minFilter, wrapU, wrapV);
                    stbi_image_free(data);
                    return newImage;
                }
                return nullptr;
            };
            const ktx_transcode_fmt_e transcodeFormat =
                              device.isFormatSupported(VK_FORMAT_BC7_SRGB_BLOCK) ? KTX_TTF_BC7_RGBA :
                              device.isFormatSupported(VK_FORMAT_BC3_SRGB_BLOCK) ? KTX_TTF_BC3_RGBA :
                              device.isFormatSupported(VK_FORMAT_BC1_RGBA_SRGB_BLOCK) ? KTX_TTF_BC1_OR_3 :
                              KTX_TTF_RGBA32;
            auto loadImageKTX2 = [&](const string&  name,
                                     const void   * srcData,
                                     const size_t   size,
                                     const VkFormat format) -> shared_ptr<Image> {
                if (!device.getDeviceFeatures().textureCompressionBC) {
                    die("GPU does not support BC texture compression");
                }
                ktxTexture2* texture;
                if (KTX_SUCCESS != ktxTexture2_CreateFromMemory(
                    static_cast<const ktx_uint8_t*>(srcData),
                    size,
                    KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                    &texture)) {
                    die("Failed to create KTX texture from memory");
                }
                if (ktxTexture2_NeedsTranscoding(texture)) {
                    if (KTX_SUCCESS != ktxTexture2_TranscodeBasis(texture, transcodeFormat, 0)) {
                        die("Failed to transcode KTX2 to BC");
                    }
                }
                const auto newImage = make_shared<VulkanImage>(
                    device, name, texture,
                    magFilter, minFilter, wrapU, wrapV);
                ktxTexture_Destroy((ktxTexture*)texture);
                return newImage;
            };
            auto loadTexture = [&](const fastgltf::TextureInfo& sourceTextureInfo, const VkFormat format) {
                const auto& texture = gltf.textures[sourceTextureInfo.textureIndex];
                materialsTextCoords[material->getId()] = sourceTextureInfo.texCoordIndex;
                if (texture.samplerIndex.has_value()) { convertSamplerData(texture); }
                shared_ptr<Image> image;
                if (texture.imageIndex.has_value()) {
                    const auto imageIndex = texture.imageIndex.value();
                    if (images.contains(imageIndex)) {
                        image = images[imageIndex];
                    } else {
                        image = loadImage(gltf, gltf.images[imageIndex], format, loadImageRGBA);
                        images.emplace(imageIndex, image);
                    }
                } else if (texture.basisuImageIndex.has_value()) {
                    const auto imageIndex = texture.basisuImageIndex.value();
                    if (compressedImages.contains(imageIndex)) {
                        image = compressedImages[imageIndex];
                    } else {
                        image = loadImage(gltf, gltf.images[imageIndex], format, loadImageKTX2);
                        compressedImages.emplace(imageIndex, image);
                    }
                } else {
                    die("Texture without supported image category");
                }
                if (image == nullptr) return StandardMaterial::TextureInfo{};
                auto texInfo = StandardMaterial::TextureInfo {
                    .texture = make_shared<ImageTexture>(image)
                };
                const auto& transform = sourceTextureInfo.transform;
                if (transform != nullptr) {
                    const auto translation = mat3{1,0,0, 0,1,0, transform->uvOffset[0], transform->uvOffset[1], 1};
                    const auto rotation = mat3{
                        cos(transform->rotation), sin(transform->rotation), 0,
                       -sin(transform->rotation), cos(transform->rotation), 0,
                                    0,             0, 1
                    };
                    const auto scale = mat3{transform->uvScale[0],0,0, 0,transform->uvScale[1],0, 0,0,1};
                    texInfo.transform = translation * rotation * scale;
                }
                return texInfo;
            };
            if (mat.pbrData.baseColorTexture.has_value())
                material->setAlbedoTexture(loadTexture(mat.pbrData.baseColorTexture.value(), VK_FORMAT_R8G8B8A8_SRGB));
            if (mat.pbrData.metallicRoughnessTexture.has_value()) {
                const auto& textInfo = loadTexture(mat.pbrData.metallicRoughnessTexture.value(), VK_FORMAT_R8G8B8A8_UNORM);
                material->setMetallicTexture(textInfo);
                material->setRoughnessTexture(textInfo);
            }
            if (mat.normalTexture.has_value())
                material->setNormalTexture(loadTexture(mat.normalTexture.value(), VK_FORMAT_R8G8B8A8_UNORM));
            if (mat.emissiveTexture.has_value()) {
                // https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#_material_emissivetexture
                material->setEmissiveTexture(loadTexture(mat.emissiveTexture.value(), VK_FORMAT_R8G8B8A8_SRGB));
            }
            material->setCullMode(forceBackFaceCulling      ? CULLMODE_BACK
                                          : mat.doubleSided ? CULLMODE_DISABLED
                                                            : CULLMODE_BACK);
            materials.push_back(material);
        }
        if (materials.empty()) {
            materials.push_back(std::make_shared<StandardMaterial>(""));
        }

        auto meshes = vector<shared_ptr<VulkanMesh>>(gltf.meshes.size());
        auto meshesCount = 0;
        for (size_t i = 0; i < gltf.meshes.size(); i++) {
            const auto& glftMesh = gltf.meshes[i];
            auto  mesh     = make_shared<VulkanMesh>(glftMesh.name.data());
            auto &vertices = mesh->getVertices();
            auto &indices  = mesh->getIndices();
            for (auto &&p : glftMesh.primitives) {
                auto surface = std::make_shared<Surface>(
                        static_cast<uint32_t>(indices.size()),
                        static_cast<uint32_t>(gltf.accessors[p.indicesAccessor.value()].count));
                auto initial_vtx  = vertices.size();
                auto haveTangents = false;
                // load indexes
                {
                    auto &indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                    indices.reserve(indices.size() + indexaccessor.count);
                    fastgltf::iterateAccessor<std::uint32_t>(
                            gltf, indexaccessor, [&](std::uint32_t idx) { indices.push_back(idx + initial_vtx); });
                }
                // load vertex positions
                {
                    auto &posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](const vec3 v, const size_t index) {
                        vertices[index + initial_vtx] = {
                            .position = v,
                        };
                    });
                }
                // load vertex normals
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end()) {
                    fastgltf::iterateAccessorWithIndex<glm::vec3>(
                            gltf, gltf.accessors[(*normals).accessorIndex], [&](vec3 v, size_t index) {
                                vertices[index + initial_vtx].normal = v;
                            });
                }
                auto tangents = p.findAttribute("TANGENT");
                if (tangents != p.attributes.end()) {
                    haveTangents = true;
                    fastgltf::iterateAccessorWithIndex<glm::vec4>(
                            gltf, gltf.accessors[(*tangents).accessorIndex], [&](vec4 v, size_t index) {
                                vertices[index + initial_vtx].tangent = v;
                            });
                }
                if (p.materialIndex.has_value()) {
                    // associate material to surface and keep track of all materials used in the Mesh
                    auto material     = materials[p.materialIndex.value()];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                    // load UVs
                    auto textCoord = 0;
                    if (materialsTextCoords.contains(material->getId())) {
                        textCoord = materialsTextCoords.at(material->getId());
                    }
                    stringstream stextCoord;
                    stextCoord << "TEXCOORD_" << textCoord;
                    auto uv = p.findAttribute(stextCoord.str());
                    if (uv != p.attributes.end()) {
                        fastgltf::iterateAccessorWithIndex<glm::vec2>(
                                gltf, gltf.accessors[(*uv).accessorIndex], [&](vec2 v, size_t index) {
                                    vertices[index + initial_vtx].uv = {v.x, v.y};
                                });
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material = make_shared<StandardMaterial>();
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
        log("Loader :", to_string(materials.size()), "materials,",
            to_string(images.size()), "images,",
            to_string(meshes.size()), "meshes (", to_string(meshesCount), "uniques)");

        // load all nodes and their meshes
        vector<shared_ptr<Node>> nodes;
        for (fastgltf::Node &node : gltf.nodes) {
            shared_ptr<Node> newNode;
            string           name{node.name.data()};
            // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the
            // MeshInstance class
            if (node.meshIndex.has_value()) {
                auto mesh = meshes[*node.meshIndex];
                mesh->buildModel();
                newNode = std::make_shared<MeshInstance>(mesh, name);
            } else {
                newNode = std::make_shared<Node>(name);
            }

            visit(fastgltf::visitor{
                          [&](fastgltf::math::fmat4x4 matrix) {
                              memcpy(&newNode->_getTransformLocal(), matrix.data(), sizeof(matrix));
                          },
                          [&](fastgltf::TRS transform) {
                              const vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
                              const quat rot(transform.rotation[3],
                                       transform.rotation[0],
                                       transform.rotation[1],
                                       transform.rotation[2]);
                              const vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);
                              const mat4 tm = translate(mat4(1.f), tl);
                              const mat4 rm = toMat4(rot);
                              const mat4 sm = scale(mat4(1.f), sc);
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
        auto rootNode = make_shared<Node>(filepath);
        for (auto &node : nodes) {
            if (node->getParent() == nullptr) {
                node->setProcessMode(PROCESS_MODE_DISABLED);
                rootNode->addChild(node);
            }
        }

        return rootNode;
    }

    void Loader::addNode(Node *parent, map<string, shared_ptr<Node>> &nodeTree, map<string, SceneNode> &sceneTree,
                         const SceneNode &nodeDesc, const bool forceBackFaceCulling) {
        constexpr auto log_name{"Scene loader :"};
        if (nodeTree.contains(nodeDesc.id))
            die(log_name, "Node with id", nodeDesc.id, "already exists in the scene tree");
        sceneTree[nodeDesc.id] = nodeDesc;
        shared_ptr<Node> node;
        if (nodeDesc.isResource) {
            if (nodeDesc.resourceType == "model") {
                // the model is in a glTF file
                node = loadModelFromFile(nodeDesc.resource, forceBackFaceCulling);
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
                    addNode(node.get(), nodeTree, sceneTree, child, forceBackFaceCulling);
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

    void Loader::addSceneFromFile(const shared_ptr<Node> &parent, const string &filepath, const bool forceBackFaceCulling) {
        addSceneFromFile(parent.get(), filepath, forceBackFaceCulling);
    }

    void Loader::addSceneFromFile(Node *parent, const string &filepath, const bool forceBackFaceCulling) {
        map<string, shared_ptr<Node>> nodeTree;
        map<string, SceneNode>        sceneTree;
        for (const auto &nodeDesc : loadSceneDescriptionFromJSON(filepath)) {
            addNode(parent, nodeTree, sceneTree, nodeDesc, forceBackFaceCulling);
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
            auto jsonData = nlohmann::ordered_json::parse(VirtualFS::openFile(filepath)); // parsing using ordered_json to preserver fields order
            if (jsonData.contains("includes")) {
                const vector<string> includes = jsonData["includes"];
                for (const auto &include : includes) {
                    vector<SceneNode> includeNodes = loadSceneDescriptionFromJSON(VirtualFS::parentPath(filepath) + include);
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
