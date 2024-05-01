#pragma once

#include "z0/device.h"
#include "z0/resources/resource.h"

#include <filesystem>

namespace z0 {

    class Image: public Resource {
    public:
        virtual ~Image();

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }
        vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        // Load image from file, relative to the application directory
        static shared_ptr<Image> loadFromFile(const string& filepath);

    private:
        const Device& device;
        uint32_t width, height, mipLevels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();
        void generateMipmaps(VkFormat imageFormat);

    public:
        VkDescriptorImageInfo _getImageInfo() const;

        explicit Image(const Device& device,
                       const filesystem::path& filepath,
                       uint32_t width,
                       uint32_t height,
                       const VkDeviceSize imageSize,
                       const void* data,
                       const VkFormat format);
    };

}