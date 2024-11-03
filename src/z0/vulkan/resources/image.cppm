/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"
#include <ktx.h>

export module z0:VulkanImage;

import :Image;

import :Device;

export namespace z0 {

    /**
     * Helper class for the Vulkan resources [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html), [VkImageView](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageView.html) and [VkSampler](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSampler.html)
     */
    class VulkanImage : public Image {
    public:
        VulkanImage(const Device &  device,
               const string &       name,
               uint32_t             width,
               uint32_t             height,
               VkDeviceSize         imageSize,
               const void *         data,
               VkFormat             format             = VK_FORMAT_R8G8B8A8_SRGB,
               VkImageTiling        tiling             = VK_IMAGE_TILING_OPTIMAL,
               VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
               VkFilter             samplerFilter      = VK_FILTER_LINEAR,
               bool                 noMipmaps          = false);

        VulkanImage(const Device &  device,
               const string &       name,
               uint32_t             width,
               uint32_t             height,
               VkDeviceSize         imageSize,
               const void *         data,
               VkFormat             format,
               VkFilter             magFiter,
               VkFilter             minFiler,
               VkSamplerAddressMode samplerAddressModeU,
               VkSamplerAddressMode samplerAddressModeV,
               VkImageTiling        tiling             = VK_IMAGE_TILING_OPTIMAL,
               bool                 noMipmaps          = false);

        VulkanImage(const Device &  device,
               const string &       name,
               const ktxTexture2*   kTexture,
               VkFormat             format,
               VkFilter             magFiter,
               VkFilter             minFiler,
               VkSamplerAddressMode samplerAddressModeU,
               VkSamplerAddressMode samplerAddressModeV,
               VkImageTiling        tiling             = VK_IMAGE_TILING_OPTIMAL);

        VulkanImage(const Device &  device,
               uint32_t             width,
               uint32_t             height,
               uint32_t             layers,
               VkFormat             format,
               uint32_t             numMipLevels,
               VkImageUsageFlags    additionalUsage,
               VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT ,
               VkFilter             samplerFilter      = VK_FILTER_LINEAR,
               VkBool32             anisotropyEnable   = VK_TRUE);

        ~VulkanImage() override;

        inline VkDescriptorImageInfo getImageInfo() const {
            // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors
            return VkDescriptorImageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        [[nodiscard]] inline VkImage getImage() const { return textureImage; }

        [[nodiscard]] inline VkImageView getImageView() const { return textureImageView; }

    private:
        const Device & device;
        uint32_t       mipLevels;
        VkImage        textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView    textureImageView;
        VkSampler      textureSampler;

        void createTextureSampler(
            VkFilter magFilter,
            VkFilter minFilter,
            VkSamplerAddressMode samplerAddressModeU,
            VkSamplerAddressMode samplerAddressModeV,
            VkBool32 anisotropyEnable = VK_TRUE);

        inline void createTextureSampler(
            const VkFilter filter,
            const VkSamplerAddressMode samplerAddressMode,
            const VkBool32 anisotropyEnable = VK_TRUE) {
            createTextureSampler(filter, filter, samplerAddressMode, samplerAddressMode, anisotropyEnable);
        }

        void generateMipmaps(VkFormat imageFormat) const;
    };

}
