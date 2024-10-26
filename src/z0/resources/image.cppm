module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:Image;

import :Resource;
import :Device;

export namespace z0 {

    /**
     * A bitmap resource, stored in GPU memory.
     * Use loadFromFile(const string& filepath) to load a image from a file.
     * Helper class for the Vulkan resources [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html), [VkImageView](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageView.html) and [VkSampler](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSampler.html)
     */
    class Image : public Resource {
    public:
        ~Image() override;

        /**
         * Returns the width in pixels
         */
        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels
         */
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        /**
         * Returns the size in pixels
         */
        [[nodiscard]] inline vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        /**
         * Load a bitmap from file. For supported file formats see https://github.com/nothings/stb/blob/master/stb_image.h
         * @param filename : path of the file, relative to the application working directory
         */
        [[nodiscard]] static shared_ptr<Image> loadFromFile(const string &filename);

        [[nodiscard]] static unique_ptr<Image> createBlankImage();

    private:
        const Device & device;
        uint32_t       width;
        uint32_t       height;
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

    public:
        inline VkDescriptorImageInfo _getImageInfo() const {
            // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler#page_Updating-the-descriptors
            return VkDescriptorImageInfo{
                .sampler = textureSampler,
                .imageView = textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }

        [[nodiscard]] inline VkImage _getImage() const { return textureImage; }
        [[nodiscard]] inline VkImageView _getImageView() const { return textureImageView; }

        Image(const Device &        device,
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

        Image(const Device &        device,
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

        Image(const Device &       device,
               uint32_t             width,
               uint32_t             height,
               uint32_t             layers,
               VkFormat             format,
               uint32_t             numMipLevels,
               VkImageUsageFlags    additionalUsage,
               VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT ,
               VkFilter             samplerFilter      = VK_FILTER_LINEAR,
               VkBool32             anisotropyEnable   = VK_TRUE
               );


    };

}
