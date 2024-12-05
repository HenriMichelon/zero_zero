/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.resources.Cubemap;

import z0.Constants;
import z0.Tools;

import z0.resources.Image;
import z0.resources.Resource;

export namespace z0 {

    /**
     * %A cubemap composed by six images stored in a single [VkImage](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkImage.html) in GPU memory.
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
            uint32_t             width,
            uint32_t             height,
            uint32_t             imageSize,
            const vector<byte*> &data,
            const string &       name = "Cubemap");

        ~Cubemap() override = default;

        /**
         * Loads a cubemap from 6 RGBA images files.
         * Images must be named `{name}_back.{ext}`, `{name}_front.{ext}`, `{name}_top.{ext}`, `{name}_bottom.{ext}`, `{name}_left.{ext}` and `{name}_right.{ext}`
         * and **must** have the same sizes
         * @param filepath path and filename (without the extension) of the images
         * @param fileext files extension
         * @param imageFormat format
         */
        static shared_ptr<Cubemap> load(const string &filepath, const string &fileext, ImageFormat imageFormat = ImageFormat::R8G8B8A8_SRGB);

        /**
         * Loads the cubemap from a single RGBA image with the following format :<br>
         *&emsp;&emsp;&emsp;`top`<br>
         *&emsp;`left  front  right  back`<br>
         *&emsp;&emsp;&emsp;`bottom`<br>
         */
        static shared_ptr<Cubemap> load(const string &filepath, ImageFormat imageFormat = ImageFormat::R8G8B8A8_SRGB);

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

        [[nodiscard]] static byte *extractImage(const byte *source,
                                                int   x, int y,
                                                int   srcWidth,
                                                int   w, int h,
                                                int   channels);
    };

    /**
     * Environment cubemap
     */
    class EnvironmentCubemap : public Cubemap {
    public:
        static constexpr auto ENVIRONMENT_MAP_SIZE{1024};
        static inline auto ENVIRONMENT_MAP_MIPMAP_LEVELS = numMipmapLevels(ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);
        static constexpr auto IRRADIANCE_MAP_SIZE{32};
        static constexpr auto BRDFLUT_SIZE{256};

        EnvironmentCubemap(const string &  name = "EnvironmentCubemap");

        /**
          * Loads the cubemap from a single HDRi.
          */
        [[nodiscard]] static shared_ptr<EnvironmentCubemap> loadFromHDRi(const string &filename, ImageFormat imageFormat = ImageFormat::R8G8B8A8_SRGB);

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
