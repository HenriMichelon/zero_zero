module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtc/matrix_transform.hpp>
#include <volk.h>

module z0;

import :Tools;
import :Constants;
import :Light;
import :DirectionalLight;
import :SpotLight;
import :Device;
import :ShadowMapFrameBuffer;

namespace z0 {

    ShadowMapFrameBuffer::ShadowMapFrameBuffer(const Device &dev, const Light *spotLight) :
        SampledFrameBuffer{dev},
        light{spotLight},
        lightIsDirectional{dynamic_cast<const DirectionalLight *>(light) != nullptr},
        size{static_cast<uint32_t>(lightIsDirectional ? 8192 : 2048)} {
        ShadowMapFrameBuffer::createImagesResources();
    }

    mat4 ShadowMapFrameBuffer::getLightSpace(const vec3 cameraPosition) const {
        vec3 lightPosition;
        vec3 sceneCenter;
        mat4 lightProjection;
        if (lightIsDirectional) {
            const auto *directionalLight = dynamic_cast<const DirectionalLight *>(light);
            const auto lightDirection = normalize(
                    mat3{directionalLight->getTransformGlobal()} * directionalLight->getDirection());
            // Scene bounds
            constexpr auto limit    = 20.0f;
            auto           sceneMin = vec3{-limit, -limit, -limit} + cameraPosition;
            auto           sceneMax = vec3{limit, limit, limit} + cameraPosition;
            // Set up the orthographic projection matrix
            auto orthoWidth  = distance(sceneMin.x, sceneMax.x);
            auto orthoHeight = distance(sceneMin.y, sceneMax.y);
            auto orthoDepth  = distance(sceneMin.z, sceneMax.z);
            sceneCenter      = (sceneMin + sceneMax) / 2.0f;
            lightPosition    = sceneCenter - lightDirection * (orthoDepth / 2.0f);
            // Position is scene center offset by light direction
            lightProjection = ortho(-orthoWidth / 2, orthoWidth / 2,
                                    -orthoHeight / 2, orthoHeight / 2,
                                    -orthoDepth/4, orthoDepth);
        } else if (auto *spotLight = dynamic_cast<const SpotLight *>(light)) {
            const auto lightDirection = normalize(mat3{spotLight->getTransformGlobal()} * spotLight->getDirection());
            lightPosition       = light->getPositionGlobal();
            sceneCenter         = lightPosition + lightDirection;
            lightProjection     = perspective(spotLight->getFov(), device.getAspectRatio(), 0.1f, 20.0f);
        } else {
            return mat4{};
        }
        // Combine the projection and view matrix to form the light's space matrix
        return lightProjection * lookAt(lightPosition, sceneCenter, AXIS_UP);
    }

    void ShadowMapFrameBuffer::createImagesResources() {
        // https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmapping/shadowmapping.cpp#L192
        // For shadow mapping we only need a depth attachment
        const auto format = device.findImageTilingSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT,},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createImage(size,
                    size,
                    format,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_IMAGE_ASPECT_DEPTH_BIT);
        // Create sampler to sample from to depth attachment
        // Used to sample in the fragment shader for shadowed rendering
        const VkFilter shadowmap_filter = device.formatIsFilterable(format, VK_IMAGE_TILING_OPTIMAL)
                ? VK_FILTER_LINEAR
                : VK_FILTER_NEAREST;
        const VkSamplerCreateInfo samplerCreateInfo{
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

    void ShadowMapFrameBuffer::cleanupImagesResources() {
        if (sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device.getDevice(), sampler, nullptr);
            sampler = VK_NULL_HANDLE;
        }
        FrameBuffer::cleanupImagesResources();
    }

}
