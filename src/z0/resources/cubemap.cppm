module;
#include "z0/libraries.h"

export module z0:Cubemap;

import :Tools;
import :Resource;
import :Image;

export namespace z0 {

    /**
     * A cubemap composed by six images stored in a single [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html) in GPU memory.
     * Use loadFromFile(const string &filepath, const string &ext) to load the cubemap
     */
    class Cubemap : public Resource {
    public:
        enum Type {
            TYPE_STANDARD    = 0,
            TYPE_ENVIRONMENT = 1
        };

        /**
         * Creates a Cubemap from 6 images stored in CPU memory.
         * The images **must** have the same sizes
         * @param width : width in pixels of the images
         * @param height : height in pixels of the images
         * @param imageSize : size of each image in bytes
         * @param data : 6 images datas in this order : right, left, top, bottom, front, back
         * @param name : resource name
         */
        static shared_ptr<Cubemap> create(
            uint32_t                       width,
            uint32_t                       height,
            uint32_t                       imageSize,
            const vector<unsigned char *> &data,
            const string &                 name = "Cubemap");

        ~Cubemap() override = default;

        /**
         * Loads a cubemap from 6 RGBA images files.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filename path and filename (without the extension) of the images
         * @param fileext files extension
         */
        static shared_ptr<Cubemap> loadFromFile(const string &filename, const string &fileext);

        /**
         * Loads the cubemap from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  front  right  back`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         * @param filename path of the image
         */
        static shared_ptr<Cubemap> loadFromFile(const string &filename);

        /**
         * Returns the width in pixels of each image
         */
        [[nodiscard]] inline uint32_t getWidth() const { return width; }

        /**
         * Returns the height in pixels of each image
         */
        [[nodiscard]] inline uint32_t getHeight() const { return height; }

        [[nodiscard]] static shared_ptr<Cubemap> createBlankCubemap();

        [[nodiscard]] Type getCubemapType() const { return type; }

    protected:
        Type    type;
        uint32_t width;
        uint32_t height;

        Cubemap(uint32_t      width,
                uint32_t      height,
                Type          type = TYPE_STANDARD,
                const string &name = "Cubemap");

        [[nodiscard]] static unsigned char *extractImage(unsigned char *source,
                                                         int            x, int y,
                                                         int            srcWidth,
                                                         int            w, int h,
                                                         int            channels);
    };

    /**
     * Environment cubemap
     */
    class EnvironmentCubemap : public Cubemap {
    public:
        static constexpr auto ENVIRONMENT_MAP_SIZE{1024};
        static constexpr auto ENVIRONMENT_MAP_MIPMAP_LEVELS = numMipmapLevels(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);
        static constexpr auto IRRADIANCE_MAP_SIZE{32};
        static constexpr auto BRDFLUT_SIZE{256};

        EnvironmentCubemap(const string &  name = "EnvironmentCubemap");

        /**
          * Loads the cubemap from a single HDRi.
          * @param filename path of the image
          */
        [[nodiscard]] static shared_ptr<EnvironmentCubemap> loadFromHDRi(const string &filename);

        [[nodiscard]] inline shared_ptr<Cubemap> getSpecularCubemap() const { return specularCubemap; }
        [[nodiscard]] inline shared_ptr<Cubemap> getIrradianceCubemap() const { return irradianceCubemap; }
        [[nodiscard]] inline shared_ptr<Image> getBRDFLut() const { return brdfLut; }

    private:
        // Specular map
        shared_ptr<Cubemap> specularCubemap;
        // Irradiance map
        shared_ptr<Cubemap> irradianceCubemap;
        // 2D LUT for split-sum approximation
        shared_ptr<Image> brdfLut;
    };

}
