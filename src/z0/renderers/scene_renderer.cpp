/*
 * Using one descriptor per scene with offsets
 * https://docs.vulkan.org/samples/latest/samples/performance/descriptor_management/README.html
 */
#include "z0/renderers/scene_renderer.h"

#include <stb_image_write.h>

#include <algorithm>

namespace z0 {

    void stb_write_func(void *context, void *data, int size) {
        auto* buffer = reinterpret_cast<vector<unsigned char>*>(context);
        auto* ptr = static_cast<unsigned char*>(data);
        buffer->insert(buffer->end(), ptr, ptr + size);
    }

    SceneRenderer::SceneRenderer(const Device &dev, const string& sDir) :
            BaseModelsRenderer{dev, sDir},
            colorFrameBufferMultisampled{dev, true} {
     }

    void SceneRenderer::cleanup() {
        if (blankImage != nullptr) {
            blankImage.reset();
            blankImageData.clear();
        }
        opaquesModels.clear();
        BaseModelsRenderer::cleanup();
    }

    void SceneRenderer::addingModel(MeshInstance *meshInstance, uint32_t modelIndex) {
        opaquesModels.push_back(meshInstance);
        modelsIndices[meshInstance->getId()] = modelIndex;
        for (const auto &material: meshInstance->getMesh()->_getMaterials()) {
            if (find(materials.begin(), materials.end(), material.get()) != materials.end()) continue;
            materialsIndices[material->getId()] = materials.size();
            materials.push_back(material.get());
            if (auto *standardMaterial = dynamic_cast<StandardMaterial *>(material.get())) {
                if (standardMaterial->getAlbedoTexture() != nullptr) addImage(standardMaterial->getAlbedoTexture()->getImage());
                if (standardMaterial->getSpecularTexture() != nullptr) addImage(standardMaterial->getSpecularTexture()->getImage());
                if (standardMaterial->getNormalTexture() != nullptr) addImage(standardMaterial->getNormalTexture()->getImage());
            }
        }
    }

    void SceneRenderer::addImage(const shared_ptr<Image>& image) {
        if (find(images.begin(), images.end(), image.get()) != images.end()) return;
        if (images.size() == MAX_IMAGES) die("Maximum images count reached for the scene renderer");
        imagesIndices[image->getId()] = static_cast<int32_t>(images.size());
        images.push_back(image.get());
    }

    void SceneRenderer::removingModel(z0::MeshInstance *meshInstance) {
        auto it = find(opaquesModels.begin(), opaquesModels.end(), meshInstance);
        if (it != opaquesModels.end()) {
            opaquesModels.erase(it);
            modelsIndices.erase(meshInstance->getId());
        }
    }

    void SceneRenderer::loadShaders() {
        vertShader = createShader("default.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader = createShader("default.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void SceneRenderer::update(uint32_t currentFrame) {
        if (currentCamera == nullptr) return;
        if (models.empty()) return;

        GobalUniformBuffer globalUbo{
            .projection = currentCamera->getProjection(),
            .view = currentCamera->getView(),
            .cameraPosition = currentCamera->getPosition(),
        };
        writeUniformBuffer(globalUniformBuffers, currentFrame, &globalUbo);

        uint32_t modelIndex = 0;
        for (const auto& meshInstance: models) {
            ModelUniformBuffer modelUbo {
                .matrix = meshInstance->getTransformGlobal(),
            };
            writeUniformBuffer(modelUniformBuffers, currentFrame, &modelUbo, modelIndex);
            modelIndex += 1;
        }

        uint32_t materialIndex = 0;
        for (auto *material: materials) {
            if (material->_isUpdated()) {
                material->_resetUpdate();
                MaterialUniformBuffer materialUbo{};
                if (auto *standardMaterial = dynamic_cast<StandardMaterial *>(material)) {
                    materialUbo.albedoColor = standardMaterial->getAlbedoColor().color;
                    if (standardMaterial->getAlbedoTexture() != nullptr) {
                        materialUbo.diffuseIndex = imagesIndices[standardMaterial->getAlbedoTexture()->getImage()->getId()];
                    }
                    if (standardMaterial->getSpecularTexture() != nullptr) {
                        materialUbo.specularIndex = imagesIndices[standardMaterial->getSpecularTexture()->getImage()->getId()];
                    }
                    if (standardMaterial->getNormalTexture() != nullptr) {
                        materialUbo.normalIndex = imagesIndices[standardMaterial->getNormalTexture()->getImage()->getId()];
                    }
                    materialUbo.transparency = standardMaterial->getTransparency();
                    materialUbo.alphaScissor = standardMaterial->getAlphaScissor();
                }
                writeUniformBuffer(materialsUniformBuffers, currentFrame, &materialUbo, materialIndex);
            }
            materialIndex += 1;
        }
    }

    void SceneRenderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) {
        if (currentCamera == nullptr) return;
        if (models.empty()) return;
        setInitialState(commandBuffer);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        drawModels(commandBuffer, currentFrame, opaquesModels);
    }

    void SceneRenderer::drawModels(VkCommandBuffer commandBuffer, uint32_t currentFrame, const list<MeshInstance*>& modelsToDraw) {
        for (const auto& meshInstance : modelsToDraw) {
            if (meshInstance->isValid()) {
                const auto& modelIndex = modelsIndices[meshInstance->getId()];
                const auto& model = meshInstance->getMesh();
                for (const auto& surface: model->getSurfaces()) {
                    const auto& materialIndex = materialsIndices[surface->material->getId()];
                    array<uint32_t, 3> offsets = {
                        0, // globalBuffers
                        static_cast<uint32_t>(modelUniformBuffers[currentFrame]->getAlignmentSize() * modelIndex),
                        static_cast<uint32_t>(materialsUniformBuffers[currentFrame]->getAlignmentSize() * materialIndex),
                    };
                    vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);
                    bindDescriptorSets(commandBuffer, currentFrame, offsets.size(), offsets.data());
                    model->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                }
            }
        }
    }

    void SceneRenderer::createDescriptorSetLayout() {
        if (currentCamera == nullptr) return;
        if (models.empty()) return;
        descriptorPool = DescriptorPool::Builder(device)
                .setMaxSets(MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // global UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // models UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_FRAMES_IN_FLIGHT) // materials UBO
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT) // images textures
                .build();

        setLayout = DescriptorSetLayout::Builder(device)
                .addBinding(0, // global UBO
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                            VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, // models UBO
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                            VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(2, // materials UBO
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                            VK_SHADER_STAGE_FRAGMENT_BIT)
                .addBinding(3, // images textures
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            VK_SHADER_STAGE_FRAGMENT_BIT,
                            MAX_IMAGES)
                .build();

        globalUniformBufferSize = sizeof(GobalUniformBuffer);

        // Create an in-memory default blank image
        // and initialize the image info array with this image
        if (blankImage == nullptr) {
            auto data = new unsigned char[1 * 1 * 3];
            data[0] = 0;
            data[1] = 0;
            data[2] = 0;
            stbi_write_jpg_to_func(stb_write_func, &blankImageData, 1, 1, 3, data, 100);
            delete[] data;
            blankImage = make_shared<Image>(device, "Blank", 1, 1, blankImageData.size(), blankImageData.data());
            for (auto &imageInfo: imagesInfo) {
                imageInfo = blankImage->_getImageInfo();
            }
        }

        createUniformBuffers(globalUniformBuffers, globalUniformBufferSize);
        createOrUpdateDescriptorSet(true);
    }

    void SceneRenderer::createOrUpdateDescriptorSet(bool create) {
        if (modelUniformBufferCount != models.size()) {
            modelUniformBufferCount = models.size();
            createUniformBuffers(modelUniformBuffers, modelUniformBufferSize, modelUniformBufferCount);
        }
        if (materialUniformBufferCount != materials.size()) {
            materialUniformBufferCount = materials.size();
            createUniformBuffers(materialsUniformBuffers, materialUniformBufferSize, materialUniformBufferCount);
        }

        uint32_t imageIndex = 0;
        for(const auto& image : images) {
            imagesInfo[imageIndex] = image->_getImageInfo();
            imageIndex += 1;
        }

        for (uint32_t i = 0; i < descriptorSet.size(); i++) {
            auto globalBufferInfo = globalUniformBuffers[i]->descriptorInfo(globalUniformBufferSize);
            auto modelBufferInfo = modelUniformBuffers[i]->descriptorInfo(modelUniformBufferSize);
            auto materialBufferInfo = materialsUniformBuffers[i]->descriptorInfo(materialUniformBufferSize);
            auto writer = DescriptorWriter(*setLayout, *descriptorPool)
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &modelBufferInfo)
                .writeBuffer(2, &materialBufferInfo)
                .writeImage(3, imagesInfo.data());
            if (create) {
                if (!writer.build(descriptorSet[i])) die("Cannot allocate descriptor set");
            } else {
                writer.overwrite(descriptorSet[i]);
            }
        }
    }

    void SceneRenderer::recreateImagesResources() {
        cleanupImagesResources();
        colorFrameBufferHdr->createImagesResources();
        colorFrameBufferMultisampled.createImagesResources();
        if (depthFrameBuffer != nullptr) {
            resolvedDepthFrameBuffer->createImagesResources();
        }
    }

    void SceneRenderer::createImagesResources() {
        colorFrameBufferHdr = make_shared<ColorFrameBufferHDR>(device);
        if (depthFrameBuffer == nullptr) {
            depthFrameBuffer = make_shared<DepthFrameBuffer>(device, true);
            resolvedDepthFrameBuffer = make_shared<DepthFrameBuffer>(device, false);
        } else {
            depthFrameBuffer->createImagesResources();
        }
    }

    void SceneRenderer::cleanupImagesResources() {
        if (depthFrameBuffer != nullptr) {
            resolvedDepthFrameBuffer->cleanupImagesResources();
        }
        colorFrameBufferHdr->cleanupImagesResources();
        colorFrameBufferMultisampled.cleanupImagesResources();
    }

    // https://lesleylai.info/en/vk-khr-dynamic-rendering/
    void SceneRenderer::beginRendering(VkCommandBuffer commandBuffer) {
        Device::transitionImageLayout(commandBuffer, colorFrameBufferMultisampled.getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer, colorFrameBufferHdr->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                     0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        // Color attachement : where the rendering is done (multisampled memory image)
        // Resolved into a non multisampled image
        const VkRenderingAttachmentInfo colorAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = colorFrameBufferMultisampled.getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT ,
                .resolveImageView = colorFrameBufferHdr->getImageView(),
                .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearColor,
        };
        const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthFrameBuffer->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
                .resolveImageView = resolvedDepthFrameBuffer->getImageView(),
                .resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = depthClearValue,
        };
        const VkRenderingInfo renderingInfo{
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .pNext = nullptr,
                .renderArea = {{0, 0}, device.getSwapChainExtent()},
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo,
                .pStencilAttachment = nullptr
        };
        vkCmdBeginRendering(commandBuffer, &renderingInfo);
    }

    void SceneRenderer::endRendering(VkCommandBuffer commandBuffer, bool isLast) {
        vkCmdEndRendering(commandBuffer);
        Device::transitionImageLayout(commandBuffer, colorFrameBufferHdr->getImage(),
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                           isLast ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                     0,
                                           isLast ? VK_ACCESS_TRANSFER_READ_BIT : VK_ACCESS_SHADER_READ_BIT,
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           isLast ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
        Device::transitionImageLayout(commandBuffer, resolvedDepthFrameBuffer->getImage(),
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
                                           0,
                                           VK_ACCESS_SHADER_READ_BIT,
                                           VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                           VK_IMAGE_ASPECT_DEPTH_BIT);
    }



}