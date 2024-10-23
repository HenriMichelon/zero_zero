module;
#include <cstdlib>
#include <volk.h>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Tools;
import :Resource;
import :Renderer;
import :Renderpass;
import :Device;
import :ShadowMapFrameBuffer;
import :Descriptors;
import :MeshInstance;
import :Light;
import :OmniLight;
import :SpotLight;
import :Buffer;
import :Mesh;
import :ColorFrameBufferHDR;
import :ShadowMapRenderer;
import :FrustumCulling;

namespace z0 {

    ShadowMapRenderer::ShadowMapRenderer(Device &device, const string &shaderDirectory, const Light *light) :
        Renderpass{device, shaderDirectory, WINDOW_CLEAR_COLOR},
        light{light} {
        frameData.resize(device.getFramesInFlight());
        for(auto& frame: frameData) {
            frame.shadowMap = make_shared<ShadowMapFrameBuffer>(device, isCascaded(), isCubemap());
            if (isCascaded()) { frame.cascadesCount = reinterpret_cast<const DirectionalLight*>(light)->getShadowMapCascadesCount(); }
        }
    }

    void ShadowMapRenderer::loadScene(const list<MeshInstance *> &meshes) {
        for(auto& frame: frameData) {
            frame.models = meshes;
            frame.models.sort([](const MeshInstance *a, const MeshInstance *b) { return *a < *b; });
        }
        createOrUpdateResources(false, &pushConstantRange);
    }

    void ShadowMapRenderer::cleanup() {
        cleanupImagesResources();
        for(auto& frame: frameData) {
            frame.shadowMap.reset();
        }
        Renderpass::cleanup();
    }

    ShadowMapRenderer::~ShadowMapRenderer() { ShadowMapRenderer::cleanup(); }

    void ShadowMapRenderer::update(const uint32_t currentFrame) {
        auto& data = frameData[currentFrame];
        if (data.currentCamera == nullptr) { return; }
        switch (light->getLightType()) {
            case Light::LIGHT_DIRECTIONAL: {
                // https://www.saschawillems.de/blog/2017/12/30/new-vulkan-example-cascaded-shadow-mapping/
                // https://johanmedestrom.wordpress.com/2016/03/18/opengl-cascaded-shadow-maps/
                // https://learnopengl.com/Guest-Articles/2021/CSM

                auto cascadeSplits = vector<float>(data.cascadesCount);
                const auto *directionalLight = reinterpret_cast<const DirectionalLight *>(light);
                const auto lightDirection = directionalLight->getFrontVector();
                const auto nearClip  = data.currentCamera->getNearDistance();
                const auto farClip   = data.currentCamera->getFarDistance();
                const auto clipRange = farClip - nearClip;
                const auto minZ = nearClip;
                const auto maxZ = nearClip + clipRange;
                const auto range = maxZ - minZ;
                const auto ratio = maxZ / minZ;

                // Calculate split depths based on view camera frustum
                // Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
                for (auto i = 0; i < data.cascadesCount; i++) {
                    float p          = (i + 1) / static_cast<float>(data.cascadesCount);
                    float log        = minZ * std::pow(ratio, p);
                    float uniform    = minZ + range * p;
                    float d          = cascadeSplitLambda * (log - uniform) + uniform;
                    cascadeSplits[i] = (d - nearClip) / clipRange;
                }

                // Calculate orthographic projection matrix for each cascade
                float lastSplitDist = 0.0;
                const auto invCam = inverse(data.currentCamera->getProjection() * data.currentCamera->getView());
                for (auto cascadeIndex = 0; cascadeIndex < data.cascadesCount; cascadeIndex++) {
                    const auto splitDist = cascadeSplits[cascadeIndex];

                    // Camera frustum corners in NDC space
                    vec3 frustumCorners[] = {
                        vec3(-1.0f, 1.0f, -1.0f),
                        vec3(1.0f, 1.0f, -1.0f),
                        vec3(1.0f, -1.0f, -1.0f),
                        vec3(-1.0f, -1.0f, -1.0f),

                        vec3(-1.0f, 1.0f, 1.0f),
                        vec3(1.0f, 1.0f, 1.0f),
                        vec3(1.0f, -1.0f, 1.0f),
                        vec3(-1.0f, -1.0f, 1.0f),
                    };

                    // Camera frustum corners into world space
                    for (auto j = 0; j < 8; j++) {
                        const auto invCorner = invCam * vec4(frustumCorners[j], 1.0f);
                        frustumCorners[j]   = invCorner / invCorner.w;
                    }

                    // Adjust the coordinates of near and far planes for this specific cascade
                    for (auto j = 0; j < 4; j++) {
                        const auto dist = frustumCorners[j + 4] - frustumCorners[j];
                        frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
                        frustumCorners[j]     = frustumCorners[j] + (dist * lastSplitDist);
                    }

                    // Frustum center for this cascade split, in world space
                    auto frustumCenter = VEC3ZERO;
                    for (auto j = 0; j < 8; j++) {
                        frustumCenter += frustumCorners[j];
                    }
                    frustumCenter /= 8.0f;

                    // Radius of the cascade split
                    auto radius = 0.0f;
                    for (auto j = 0; j < 8; j++) {
                        const auto distance = length(frustumCorners[j] - frustumCenter);
                        radius         = glm::max(radius, distance);
                    }
                    radius = std::ceil(radius * 16.0f) / 16.0f;

                    // Snap the frustum center to the nearest texel grid
                    const auto shadowMapResolution = static_cast<float>(data.shadowMap->getWidth());
                    const float worldUnitsPerTexel = (2.0f * radius) / shadowMapResolution;
                    frustumCenter.x = std::floor(frustumCenter.x / worldUnitsPerTexel) * worldUnitsPerTexel;
                    frustumCenter.y = std::floor(frustumCenter.y / worldUnitsPerTexel) * worldUnitsPerTexel;
                    frustumCenter.z = std::floor(frustumCenter.z / worldUnitsPerTexel) * worldUnitsPerTexel;

                    // Split bounding box
                    const auto maxExtents = vec3(radius);
                    const auto minExtents = -maxExtents;
                    const auto depth = maxExtents.z - minExtents.z;

                    // View & projection matrices
                    const auto eye = frustumCenter - lightDirection * -minExtents.z ;
                    const auto viewMatrix = lookAt(eye, frustumCenter, AXIS_UP);
                    auto lightProjection = ortho(
                        minExtents.x, maxExtents.x,
                        minExtents.y, maxExtents.y,
                        -depth, depth);

                    // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering
                    // Create the rounding matrix, by projecting the world-space origin and determining
                    // the fractional offset in texel space
                    const auto shadowMatrix = lightProjection * viewMatrix;
                    const auto shadowOrigin = (shadowMatrix * vec4{0.0f, 0.0f, 0.0f, 1.0f}) * (shadowMapResolution / 2.0f);
                    const auto roundedOrigin = round(shadowOrigin);
                    auto roundOffset = roundedOrigin - shadowOrigin;
                    roundOffset = roundOffset *  2.0f / shadowMapResolution;
                    roundOffset.z = 0.0f;
                    roundOffset.w = 0.0f;
                    lightProjection[3] += roundOffset;

                    // Store the results in the frame data
                    data.lightSpace[cascadeIndex] = lightProjection * viewMatrix;
                    data.splitDepth[cascadeIndex] = (nearClip + splitDist * clipRange);
                    lastSplitDist = cascadeSplits[cascadeIndex];
                }
                break;
            }
            case Light::LIGHT_OMNI: {
                const auto *omniLight = reinterpret_cast<const OmniLight *>(light);
                const auto lightPos  = omniLight->getPositionGlobal();
                const auto lightProj = perspective(
                    radians(90.0f),
                    data.shadowMap->getRatio(),
                    omniLight->getNearClipDistance(),
                    omniLight->getFarClipDistance());
                data.lightSpace[0] = lightProj * lookAt(lightPos, lightPos + AXIS_RIGHT, vec3(0.0,-1.0, 0.0));
                data.lightSpace[1] = lightProj * lookAt(lightPos, lightPos + AXIS_LEFT, vec3(0.0,-1.0, 0.0));
                data.lightSpace[2] = lightProj * lookAt(lightPos, lightPos + AXIS_UP, vec3(0.0, 0.0, 1.0));
                data.lightSpace[3] = lightProj * lookAt(lightPos, lightPos + AXIS_DOWN, vec3(0.0, 0.0,-1.0));
                data.lightSpace[4] = lightProj * lookAt(lightPos, lightPos + AXIS_BACK, vec3(0.0,-1.0, 0.0));
                data.lightSpace[5] = lightProj * lookAt(lightPos, lightPos + AXIS_FRONT, vec3(0.0,-1.0, 0.0));
                break;
            }
            case Light::LIGHT_SPOT: {
                const auto *spotLight = reinterpret_cast<const SpotLight *>(light);
                const auto lightDirection           = normalize(mat3{spotLight->getTransformGlobal()} * AXIS_FRONT);
                const auto lightPosition                   = light->getPositionGlobal();
                const auto sceneCenter              = lightPosition + lightDirection;
                const auto lightProjection = perspective(
                    spotLight->getFov() * 1.5f,
                    data.shadowMap->getRatio(),
                    spotLight->getNearClipDistance(),
                    spotLight->getFarClipDistance());
                data.lightSpace[0] = lightProjection * lookAt(lightPosition, sceneCenter, AXIS_UP);
                break;
            }
        }
    }

    void ShadowMapRenderer::recordCommands(const VkCommandBuffer commandBuffer, const uint32_t currentFrame) {
        const auto& data = frameData[currentFrame];
        const auto passCount = isCubemap() ? 6 : data.cascadesCount;
        auto pushConstants = PushConstants {};
        if (light->getLightType() == Light::LIGHT_OMNI) {
            const auto* omniLight = reinterpret_cast<const OmniLight*>(light);
            pushConstants.lightPosition = omniLight->getPositionGlobal();
            pushConstants.farPlane =  omniLight->getFarClipDistance();
        };
        for (int passIndex = 0; passIndex < passCount; passIndex++) {
            device.transitionImageLayout(commandBuffer,
                                                data.shadowMap->getImage(),
                                                VK_IMAGE_LAYOUT_UNDEFINED,
                                                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                0,
                                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                                                VK_IMAGE_ASPECT_DEPTH_BIT);
            const VkRenderingAttachmentInfo depthAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView   = isCascaded() || isCubemap() ? data.shadowMap->getCascadedImageView(passIndex) : data.shadowMap->getImageView(),
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue  = depthClearValue,
            };
            const VkRenderingInfo renderingInfo{.sType      = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                                                .pNext      = nullptr,
                                                .renderArea = {
                                                    {0, 0},
                                                    {data.shadowMap->getWidth(), data.shadowMap->getHeight()}
                                                },
                                                .layerCount = 1,
                                                .colorAttachmentCount = 0,
                                                .pColorAttachments    =  nullptr,
                                                .pDepthAttachment     = &depthAttachmentInfo,
                                                .pStencilAttachment   = nullptr};
            vkCmdBeginRendering(commandBuffer, &renderingInfo);

            bindShaders(commandBuffer);
            setViewport(commandBuffer, data.shadowMap->getWidth(), data.shadowMap->getHeight());
            vkCmdSetRasterizationSamplesEXT(commandBuffer, VK_SAMPLE_COUNT_1_BIT);
            constexpr VkBool32 color_blend_enables[] = {VK_FALSE};
            vkCmdSetColorBlendEnableEXT(commandBuffer, 0, 1, color_blend_enables);
            vkCmdSetAlphaToCoverageEnableEXT(commandBuffer, VK_FALSE);
            const auto vertexBinding   = Mesh::_getBindingDescription();
            const auto vertexAttribute = Mesh::_getAttributeDescription();
            vkCmdSetVertexInputEXT(commandBuffer,
                                   vertexBinding.size(),
                                   vertexBinding.data(),
                                   vertexAttribute.size(),
                                   vertexAttribute.data());
            vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthWriteEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBiasEnable(commandBuffer, VK_TRUE);
            vkCmdSetDepthBias(commandBuffer, depthBiasConstant, 0.0f, depthBiasSlope);
            vkCmdSetCullMode(commandBuffer, VK_CULL_MODE_NONE);

            pushConstants.lightSpace = data.lightSpace[passIndex];
            auto modelIndex = 0;
            auto lastMeshId = Resource::id_t{numeric_limits<uint32_t>::max()}; // Used to reduce vkCmdBindVertexBuffers & vkCmdBindIndexBuffer calls
            for (const auto &meshInstance : data.models) {
                    const auto& mesh = meshInstance->getMesh();
                    for (const auto &surface : mesh->getSurfaces()) {
                        pushConstants.matrix = meshInstance->getTransformGlobal();
                        pushConstants.transparency = surface->material->getTransparency();
                        vkCmdPushConstants(
                            commandBuffer,
                            pipelineLayout,
                            VK_SHADER_STAGE_ALL_GRAPHICS,
                            0,
                            PUSHCONSTANTS_SIZE,
                            &pushConstants);
                        if (lastMeshId == mesh->getId()) {
                            mesh->_bindlessDraw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                        } else {
                            mesh->_draw(commandBuffer, surface->firstVertexIndex, surface->indexCount);
                            lastMeshId = mesh->getId();
                        }
                    }
                modelIndex += 1;
            }

            vkCmdEndRendering(commandBuffer);
            device.transitionImageLayout(commandBuffer,
                                         data.shadowMap->getImage(),
                                         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                         VK_ACCESS_TRANSFER_WRITE_BIT,
                                         VK_ACCESS_SHADER_READ_BIT,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         // After depth writes
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         // Before depth reads in the shader
                                         VK_IMAGE_ASPECT_DEPTH_BIT);

        }
    }

    void ShadowMapRenderer::loadShaders() {
        if (isCubemap()) {
            vertShader = createShader("shadowmap_cubemap.vert", VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT);
            fragShader = createShader("shadowmap_cubemap.frag", VK_SHADER_STAGE_FRAGMENT_BIT, 0);
        } else {
            vertShader = createShader("shadowmap.vert", VK_SHADER_STAGE_VERTEX_BIT, 0);
        }
    }

    void ShadowMapRenderer::cleanupImagesResources() {
        for(auto& frame: frameData) {
            if (frame.shadowMap != nullptr) {
                frame.shadowMap->cleanupImagesResources();
            }
        }
    }

} // namespace z0
