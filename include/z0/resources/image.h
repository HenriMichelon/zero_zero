#pragma once

namespace z0 {

    class Image: public Resource {
    public:
        explicit Image(const Device& device,
                       const string& name,
                       uint32_t width,
                       uint32_t height,
                       VkDeviceSize imageSize,
                       const void* data,
                       VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
                       VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL);
        virtual ~Image();

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }
        vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        // Load image from file, relative to the application directory
        static shared_ptr<Image> loadFromFile(const string& filepath);

    private:
        const Device&   device;
        uint32_t        width;
        uint32_t        height;
        uint32_t        mipLevels;
        VkImage         textureImage;
        VkDeviceMemory  textureImageMemory;
        VkImageView     textureImageView;
        VkSampler       textureSampler;

        void createTextureSampler();
        void generateMipmaps(VkFormat imageFormat);

    public:
        VkDescriptorImageInfo _getImageInfo() const;
    };

}