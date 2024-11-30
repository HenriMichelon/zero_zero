/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.VulkanCubemap;

import z0.Cubemap;
import z0.Device;

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

        inline VkDescriptorImageInfo getImageInfo() const {
            return VkDescriptorImageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }
        [[nodiscard]] VkFormat getFormat() const { return textureFormat; }
        [[nodiscard]] inline VkImage getImage() const { return textureImage; }
        [[nodiscard]] inline VkImageView getImageView() const { return textureImageView; }

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
