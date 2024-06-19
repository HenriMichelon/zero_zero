#pragma once

namespace z0 {

    /**
     * A cubemap composed by six images stored in a single VkImage in GPU memory.
     * Use loadFromFile(const string &filepath, const string &ext) to load the cubemap
     */
    class Cubemap: public Resource {
    public:
        /**
         * Creates a Cubemap from 6 images stored in CPU memory
         * and **must** have the same sizes
         * @param device : the GPU where the image will be stored
         * @param width : width in pixels of the images
         * @param height : height in pixels of the images
         * @param imageSize : size of each image in bytes
         * @param data : 6 images datas in this order : right, left, top, bottom, front, back
         * @param name : resource name
         */
        Cubemap(const Device& device, uint32_t width, uint32_t height, VkDeviceSize imageSize, vector<unsigned char*>& data, const string& name = "");
        ~Cubemap();

        /**
         * Returns the Vulkan image resource
         */
        VkImage& getImage() { return textureImage; }

        /**
         * Returns the Vulkan image view resource
         */
        VkImageView& getImageView() { return textureImageView; }

        /**
         * Loads the cubemap from 6 RGBA images files.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filename path and filename (without the extension) of the images
         * @param filext files extension
         */
        static shared_ptr<Cubemap> loadFromFile(const string &filepath, const string &ext);

        /**
         * Loads the cubemap from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  front  right  back`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        static shared_ptr<Cubemap> loadFromFile(const string &filepath);

        /**
         * Returns the width in pixels of each image
         */
        uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels of each image
         */
        uint32_t getHeight() const { return height; }

    private:
        const Device& device;
        uint32_t width, height;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        void createTextureSampler();
        static unsigned char* extractImage(unsigned char* source, int x, int y, int srcWidth,  int w, int h, int channels);

    public:
        VkDescriptorImageInfo _getImageInfo();
    };

}