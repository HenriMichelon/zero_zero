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
        vec2 getSize() const { return glm::vec2{getWidth(), getHeight()}; }

        static shared_ptr<Image> loadFromFile(const string& filepath);

    private:
        Device& device;
        uint32_t width, height;
        uint32_t mipLevels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();
        void generateMipmaps(VkFormat imageFormat);

    public:
        VkDescriptorImageInfo _getImageInfo();

        explicit Image(Device& device,
                       const filesystem::path& filepath,
                       uint32_t width,
                       uint32_t height,
                       VkDeviceSize imageSize,
                       void* data,
                       VkFormat format);
    };

}