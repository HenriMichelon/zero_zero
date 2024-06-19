#pragma once

namespace z0 {

    /**
     * A bitmap resource, stored in GPU memory.
     * Use loadFromFile(const string& filepath) to load a image from a file.
     */
    class Image: public Resource {
    public:
        /** 
         * Creates a Image resource and store the image data in GPU memory.
         * @param device : the GPU where the image will be stored
         * @param name : resource name. Informational only.
         * @param width : width in pixels
         * @param height : height in pixels
         * @param imageSize : data size in bytes
         * @param data : the source bitmap stored in CPU memory
         * @param format : data format of the bitmap (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFormat.html)
         * @param tiling : image tiling arrangment (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImageTiling.html)
         * @param samplerAddressMode : texture sampler mode (https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
        */
        explicit Image(const Device& device,
                       const string& name,
                       uint32_t width,
                       uint32_t height,
                       VkDeviceSize imageSize,
                       const void* data,
                       VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
                       VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
                       VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                       bool noMipmaps = false);
        virtual ~Image();

        /**
         * Returns the width in pixels
         */
        inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels
         */
        inline uint32_t getHeight() const { return height; }

        /**
         * Returns the size in pixels
         */
        vec2 getSize() const { return vec2{getWidth(), getHeight()}; }

        /**
         * Load a bitmap from file. For supported file formats see https://github.com/nothings/stb/blob/master/stb_image.h
         * @param filepath : path of the file, relative to the application working directory
         */
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

        void createTextureSampler(VkSamplerAddressMode);
        void generateMipmaps(VkFormat imageFormat);

    public:
        VkDescriptorImageInfo _getImageInfo() const;
    };

}