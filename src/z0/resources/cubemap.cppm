module;
#include <cassert>
#include "stb_image.h"
#include "z0/libraries.h"
#include <volk.h>

export module z0:Cubemap;

import :Tools;
import :Resource;
import :Device;

export namespace z0 {

    /**
     * A cubemap composed by six images stored in a single [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html) in GPU memory.
     * Use loadFromFile(const string &filepath, const string &ext) to load the cubemap
     */
    class Cubemap : public Resource {
    public:
        /**
         * Creates a Cubemap from 6 images stored in CPU memory.
         * The images **must** have the same sizes
         * @param device : the GPU where the image will be stored
         * @param width : width in pixels of the images
         * @param height : height in pixels of the images
         * @param imageSize : size of each image in bytes
         * @param data : 6 images datas in this order : right, left, top, bottom, front, back
         * @param name : resource name
         */
        Cubemap(const Device &                 device,
                uint32_t                       width,
                uint32_t                       height,
                VkDeviceSize                   imageSize,
                const vector<unsigned char *> &data,
                const string &                 name = "Cubemap");

        Cubemap(const Device &                 device,
                uint32_t                       width,
                uint32_t                       height,
                const string &                 name = "Cubemap");

        ~Cubemap() override;

        /**
         * Returns the Vulkan image resource
         */
        [[nodiscard]] inline VkImage getImage() const { return textureImage; }

        /**
         * Returns the Vulkan image view resource
         */
        [[nodiscard]] inline VkImageView getImageView() const { return textureImageView; }

        /**
         * Loads a cubemap from 6 RGBA images files.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filename path and filename (without the extension) of the images
         * @param fileext files extension
         */
        [[nodiscard]] static shared_ptr<Cubemap> loadFromFile(const string &filename, const string &fileext);

        /**
         * Loads the cubemap from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  front  right  back`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        [[nodiscard]] static shared_ptr<Cubemap> loadFromFile(const string &filename);

        /**
         * Loads the cubemap from a single HDRi.
         * @param filename path of the image
         */
        [[nodiscard]] static shared_ptr<Cubemap> loadFromHDRi(const string &filename);

        /**
         * Returns the width in pixels of each image
         */
        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels of each image
         */
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        [[nodiscard]] static unique_ptr<Cubemap> createBlankCubemap();

        static constexpr auto ENVIRONMENT_MAP_SIZE{1024};
        static constexpr auto ENVIRONMENT_MAP_MIPMAP_LEVELS = numMipmapLevels(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);

    private:
        const Device & device;
        uint32_t       width, height;
        VkImage        textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView    textureImageView;
        VkSampler      textureSampler;

        void createTextureSampler();

        [[nodiscard]] static unsigned char *extractImage(unsigned char *source,
                                                         int            x, int y,
                                                         int            srcWidth,
                                                         int            w, int h,
                                                         int            channels);

    public:
        inline VkDescriptorImageInfo _getImageInfo() const {
            return VkDescriptorImageInfo{
                    .sampler = textureSampler,
                    .imageView = textureImageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
        }
    };

}
