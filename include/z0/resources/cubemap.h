#pragma once

namespace z0 {

    /**
     * A cubemap composed by six images stored in a single VkImage in GPU memory
     */
    class Cubemap: public Resource {
    public:
        ~Cubemap();

        VkImage& getImage() { return textureImage; }
        VkImageView& getImageView() { return textureImageView; }

        static shared_ptr<Cubemap> loadFromFile(const string &filepath, const string &ext);

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

    private:
        const Device& device;
        uint32_t width, height;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();

    public:
        Cubemap(const Device& device, uint32_t width, uint32_t height, VkDeviceSize imageSize, vector<void*>& data, const string& name = "");

        VkDescriptorImageInfo _getImageInfo();
    };

}