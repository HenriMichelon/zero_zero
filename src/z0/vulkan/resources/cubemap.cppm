/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

export module z0.vulkan.Cubemap;

import z0.resources.Cubemap;

import z0.vulkan.Device;

export namespace z0 {

    class VulkanCubemap : public Cubemap {
    public:
        VulkanCubemap(Device & device,
                uint32_t             width,
                uint32_t             height,
                uint32_t             imageSize,
                const vector<byte*> &data,
                const string &       name = "Cubemap");

        VulkanCubemap(const Device &device,
                uint32_t            width,
                uint32_t            height,
                uint32_t            levels = 1,
                VkFormat            format = VK_FORMAT_R16G16B16A16_SFLOAT,
                const string &      name = "Cubemap");

        VulkanCubemap(VulkanCubemap &&) = delete;
        VulkanCubemap(VulkanCubemap &) = delete;

        ~VulkanCubemap() override;

        inline auto getImageInfo() const {
            return VkDescriptorImageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }
        [[nodiscard]] inline auto getFormat() const { return textureFormat; }
        [[nodiscard]] inline auto getImage() const { return textureImage; }
        [[nodiscard]] inline auto getImageView() const { return textureImageView; }

    protected:
        const Device & device;
        VkImage        textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView    textureImageView;
        VkSampler      textureSampler;
        const VkFormat textureFormat;

    private:
        void createTextureSampler();
    };

}
