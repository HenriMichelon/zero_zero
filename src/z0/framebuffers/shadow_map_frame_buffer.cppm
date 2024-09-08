module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtc/matrix_transform.hpp>
#include <volk.h>

export module Z0:ShadowMapFrameBuffer;

import :Tools;
import :Constants;
import :Light;
import :DirectionalLight;
import :SpotLight;
import :Device;
import :SampledFrameBuffer;

export namespace z0 {

    /**
     * Offscreen frame buffer for rendering shadow, one per light
     */
    class ShadowMapFrameBuffer: public SampledFrameBuffer {
    public:
        explicit ShadowMapFrameBuffer(const Device &dev, Light* spotLight, const vec3 position) :
            SampledFrameBuffer{dev},
            light(spotLight),
            globalPosition(position) {
            ShadowMapFrameBuffer::createImagesResources();
        }

        const float zNear = 0.1f;
        const float zFar = 50.0f;
        const uint32_t size{4096};

        [[nodiscard]] mat4 getLightSpace() const {
            vec3 lightPosition;
            vec3 sceneCenter;
            mat4 lightProjection;
            if (const auto* directionalLight = dynamic_cast<DirectionalLight*>(light)) {
                auto lightDirection = normalize(mat3{directionalLight->getTransformGlobal()} * directionalLight->getDirection());
                // Scene bounds
                const auto limit = 100.0f;
                auto sceneMin = vec3{-limit, -limit, -limit} + globalPosition;
                auto sceneMax = vec3{limit, limit, limit} + globalPosition;
                // Set up the orthographic projection matrix
                auto orthoWidth = distance(sceneMin.x, sceneMax.x);
                auto orthoHeight = distance(sceneMin.y, sceneMax.y);
                auto orthoDepth = distance(sceneMin.z, sceneMax.z);
                sceneCenter = (sceneMin + sceneMax) / 2.0f;
                lightPosition = sceneCenter - lightDirection * (orthoDepth / 2.0f); // Position is scene center offset by light direction
                lightProjection = ortho(-orthoWidth / 2,
                                        orthoWidth / 2,
                                        -orthoHeight / 2,
                                        orthoHeight / 2,
                                        zNear,
                                        orthoDepth);
            } else if (auto* spotLight = dynamic_cast<SpotLight*>(light)) {
                auto lightDirection = normalize(mat3{spotLight->getTransformGlobal()} * spotLight->getDirection());
                lightPosition = light->getPositionGlobal();
                sceneCenter = lightPosition + lightDirection;
                lightProjection = perspective(spotLight->getFov(), device.getAspectRatio(), zNear, zFar);
            } else {
                return mat4{};
            }
            // Combine the projecttion and view matrix to form the light's space matrix
            return lightProjection * lookAt(lightPosition, sceneCenter, AXIS_UP);
        }

        [[nodiscard]] inline const Light* getLight() const { return light; }

        [[nodiscard]] inline vec3 getLightPosition() const { return light->getPositionGlobal(); }

        [[nodiscard]] inline const VkSampler& getSampler() const { return sampler; }

        void setGlobalPosition(const vec3 position) { globalPosition = position; }

        void createImagesResources() override {
            // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
            // For shadow mapping we only need a depth attachment
            auto format = device.findImageTilingSupportedFormat(
                    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            createImage(size,
                        size,
                        format,
                        VK_SAMPLE_COUNT_1_BIT,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_IMAGE_ASPECT_DEPTH_BIT);
            // Create sampler to sample from to depth attachment
            // Used to sample in the fragment shader for shadowed rendering
            VkFilter shadowmap_filter = device.formatIsFilterable( format, VK_IMAGE_TILING_OPTIMAL) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
            VkSamplerCreateInfo samplerCreateInfo{
                .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter = shadowmap_filter,
                .minFilter = shadowmap_filter,
                .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                .mipLodBias = 0.0f,
                .maxAnisotropy = 1.0f,
                .minLod = 0.0f,
                .maxLod = 1.0f,
                .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
            };
            if (vkCreateSampler(device.getDevice(), &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
                die("failed to create shadowmap sampler!");
            }
        }

        void cleanupImagesResources() override {
            if (sampler != VK_NULL_HANDLE) {
                vkDestroySampler(device.getDevice(), sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
            FrameBuffer::cleanupImagesResources();
        }

    private:
        Light*  light;
        vec3    globalPosition;
    };

}