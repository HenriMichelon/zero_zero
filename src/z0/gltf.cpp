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

module z0.GlTF;

import z0.Constants;
import z0.Image;
import z0.Material;
import z0.Mesh;
import z0.MeshInstance;
import z0.Node;
import z0.Resource;
import z0.Texture;
import z0.Tools;
import z0.VirtualFS;

import z0.Device;
import z0.VulkanImage;
import z0.VulkanMesh;

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
              [](auto &) {},
              [&](fastgltf::sources::URI &) {
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
                        [](auto &) {},
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

    shared_ptr<Node> GlTF::load(const string &filepath) {
        auto tStart = std::chrono::high_resolution_clock::now();
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
        map<Resource::id_t, int> materialsTexCoords;
        for (auto &mat : gltf.materials) {
            auto material = make_shared<StandardMaterial>(mat.name.data());
            material->setAlbedoColor({
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
                              device.isFormatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK) ? KTX_TTF_ASTC_4x4_RGBA :
                              device.isFormatSupported(VK_FORMAT_BC7_SRGB_BLOCK) ? KTX_TTF_BC7_RGBA :
                              device.isFormatSupported(VK_FORMAT_BC1_RGBA_SRGB_BLOCK) ? KTX_TTF_BC1_OR_3 :
                              device.isFormatSupported(VK_FORMAT_BC3_SRGB_BLOCK) ? KTX_TTF_BC3_RGBA :
                              KTX_TTF_RGBA32;
            auto loadImageKTX2 = [&](const string&  name,
                                     const void   * srcData,
                                     const size_t   size,
                                     const VkFormat format) -> shared_ptr<Image> {
                ktxTexture2* texture;
                if (KTX_SUCCESS != ktxTexture2_CreateFromMemory(
                    static_cast<const ktx_uint8_t*>(srcData),
                    size,
                    KTX_TEXTURE_CREATE_NO_FLAGS,
                    &texture)) {
                    die("Failed to create KTX texture from memory");
                }
                if (ktxTexture2_NeedsTranscoding(texture)) {
                    if (KTX_SUCCESS != ktxTexture2_TranscodeBasis(texture, transcodeFormat, 0)) {
                        die("Failed to transcode KTX2 to BC/ASTC");
                    }
                }
                const auto newImage = make_shared<KTXVulkanImage>(
                    device, name, texture,
                    magFilter, minFilter, wrapU, wrapV, format == VK_FORMAT_R8G8B8A8_SRGB);
                ktxTexture_Destroy((ktxTexture*)texture);
                return newImage;
            };
            auto loadTexture = [&](const fastgltf::TextureInfo& sourceTextureInfo, const VkFormat format) {
                const auto& texture = gltf.textures[sourceTextureInfo.textureIndex];
                materialsTexCoords[material->getId()] = sourceTextureInfo.texCoordIndex;
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
            material->setCullMode(mat.doubleSided ? CULLMODE_DISABLED : CULLMODE_BACK);
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
                        gltf, indexaccessor, [&](const uint32_t idx) {
                            indices.push_back(idx + initial_vtx);
                            // log(format("mesh {} surface {} index {}", i, mesh->getSurfaces().size(), idx + initial_vtx));
                        });
                }
                // load vertex positions
                {
                    auto &posAccessor = gltf.accessors[p.findAttribute("POSITION")->accessorIndex];
                    vertices.resize(vertices.size() + posAccessor.count);
                    fastgltf::iterateAccessorWithIndex<vec3>(gltf, posAccessor, [&](const vec3 v, const size_t index) {
                        vertices[index + initial_vtx] = {
                            .position = v,
                        };
                        // log(format("mesh {} surface {} pos {}", i, mesh->getSurfaces().size(), to_string(v)));
                    });
                }
                // load vertex normals
                {
                    auto normals = p.findAttribute("NORMAL");
                    if (normals != p.attributes.end()) {
                        auto &normalAccessor = gltf.accessors[normals->accessorIndex];
                        fastgltf::iterateAccessorWithIndex<vec3>(
                            gltf, normalAccessor, [&](const vec3 v, const size_t index) {
                                vertices[index + initial_vtx].normal = v;
                                // log(format("mesh {} surface {} norm {}", i, mesh->getSurfaces().size(), to_string(v)));
                            });
                    }
                }
                // load uv tangents
                {
                    auto tangents = p.findAttribute("TANGENT");
                    if (tangents != p.attributes.end()) {
                        haveTangents = true;
                        auto &tangentAccessor = gltf.accessors[tangents->accessorIndex];
                        fastgltf::iterateAccessorWithIndex<vec4>(
                            gltf, tangentAccessor, [&](const vec4 v, const size_t index) {
                                vertices[index + initial_vtx].tangent = v;
                            });
                    }
                }
                if (p.materialIndex.has_value()) {
                    // associate material to surface and keep track of all materials used in the Mesh
                    const auto& material     = materials[p.materialIndex.value()];
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                    // load UVs
                    auto textCoord = 0;
                    if (materialsTexCoords.contains(material->getId())) {
                        textCoord = materialsTexCoords.at(material->getId());
                    }
                    stringstream stextCoord;
                    stextCoord << "TEXCOORD_" << textCoord;
                    auto uv = p.findAttribute(stextCoord.str());
                    if (uv != p.attributes.end()) {
                        fastgltf::iterateAccessorWithIndex<vec2>(
                            gltf, gltf.accessors[(*uv).accessorIndex], [&](const vec2 v, const size_t index) {
                                vertices[index + initial_vtx].uv = v;
                                // log(format("mesh {} surface {} uvs {} uv {}", i, mesh->getSurfaces().size(), textCoord, to_string(v)));
                            });
                    }
                } else {
                    // Mesh have no material, use a default one
                    const auto &material = make_shared<StandardMaterial>();
                    surface->material = material;
                    mesh->_getMaterials().insert(material);
                }
                // calculate missing tangents
                if (!haveTangents) {
                    for (auto i = 0; i < indices.size(); i += 3) {
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
        // TODO : use scene node
        auto rootNode = make_shared<Node>(filepath);
        for (auto &node : nodes) {
            if (node->getParent() == nullptr) {
                node->setProcessMode(PROCESS_MODE_DISABLED);
                rootNode->addChild(node);
            }
        }

        auto last_time = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
        log("glTF loading time ", to_string(last_time));
        return rootNode;
    }

} // namespace z0