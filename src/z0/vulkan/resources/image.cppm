/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"
// #include <ktx.h>
// #include <ktxvulkan.h>

export module z0.vulkan.Image;

import z0.ZRes;

import z0.resources.Image;

import z0.vulkan.Buffer;
import z0.vulkan.Device;

export namespace z0 {

    /*
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
               bool                 noMipmaps          = false,
               bool                 isArray            = false);

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
               bool                 noMipmaps          = false,
               bool                 isArray            = false);

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

        VulkanImage(const Device &            device,
                VkCommandBuffer commandBuffer,
                const string &                name,
                const ZRes::ImageHeader &   imageHeader,
                const vector<ZRes::MipLevelInfo>& mipLevelHeaders,
                const ZRes::TextureHeader & textureHeader,
                const Buffer                  &buffer,
                uint64_t                      bufferOffset,
                VkImageTiling                 tiling             = VK_IMAGE_TILING_OPTIMAL);

        VulkanImage(VulkanImage&&) = delete;
        VulkanImage(VulkanImage&) = delete;

        ~VulkanImage() override;

        inline auto getImageInfo() const {
            // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors
            return VkDescriptorImageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        [[nodiscard]] inline auto getImage() const { return textureImage; }

        [[nodiscard]] inline auto getImageView() const { return textureImageView; }

        static VkFormat formatSRGB(VkFormat format, const string& name);

    protected:
        const Device & device;
        uint32_t       mipLevels;
        VkImage        textureImage{VK_NULL_HANDLE};
        VkDeviceMemory textureImageMemory{VK_NULL_HANDLE};
        VkImageView    textureImageView{VK_NULL_HANDLE};
        VkSampler      textureSampler{VK_NULL_HANDLE};

        inline VulkanImage(const Device &  device,
              const uint32_t  width,
              const uint32_t  height,
              const uint32_t  mipLevels,
              const string&   name): Image{width, height, name} , device{device}, mipLevels{mipLevels} {};

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

    // class KTXVulkanImage : public VulkanImage {
    // public:
    //     KTXVulkanImage(const Device &  device,
    //          const string &       name,
    //          ktxTexture2*         kTexture,
    //          VkFilter             magFiter,
    //          VkFilter             minFiler,
    //          VkSamplerAddressMode samplerAddressModeU,
    //          VkSamplerAddressMode samplerAddressModeV,
    //          bool                 forceSRGB          = false,
    //          VkImageTiling        tiling             = VK_IMAGE_TILING_OPTIMAL);
    //
    //     ~KTXVulkanImage() override;
    //
    //     static void initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool);
    //
    //     static void cleanup();
    //
    // private:
    //     static ktxVulkanDeviceInfo vdi;
    //     ktxVulkanTexture texture;
    // };

}
