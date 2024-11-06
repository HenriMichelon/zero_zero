/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.ZScene;

import z0.Texture;

export namespace z0 {

    /**
     * ZScene file format
     */
    class ZScene {
    public:

        struct Header {
            static constexpr char MAGIC[]{ 'Z', 'S', 'C', 'N' };
            static constexpr uint32_t VERSION{1};
            char     magic[4];
            uint32_t version{0};
            uint32_t imagesCount{0};
            uint32_t texturesCount{0};
            uint64_t headersSize;
        };

        static constexpr auto IMAGE_NAME_SIZE{64};
        struct ImageHeader {
            char     name[IMAGE_NAME_SIZE];
            uint32_t format;
            uint32_t width;
            uint32_t height;
            uint32_t mipLevels;
            uint64_t dataOffset;
            uint64_t dataSize;
        };

        struct MipLevelHeader {
            uint64_t offset;
            uint64_t size;
        };

        struct TextureHeader {
            int32_t     imageIndex;
            // float32_t   rotation;
            // vec2        offset;
            // vec2        scale;
            uint32_t    minFilter;
            uint32_t    magFilter;
            uint32_t    samplerAddressModeU;
            uint32_t    samplerAddressModeV;
        };

        [[nodiscard]] static shared_ptr<ZScene> load(const string &filename);
        [[nodiscard]] static shared_ptr<ZScene> load(ifstream &stream);

        inline const vector<shared_ptr<ImageTexture>>& getTextures() const { return textures; };

        ZScene() = default;

        static void print(const Header& header);
        static void print(const ImageHeader& header);
        static void print(const MipLevelHeader& header);
        static void print(const TextureHeader& header);

    protected:
        Header header{};
        vector<shared_ptr<ImageTexture>> textures{};

        void loadHeader(ifstream& stream);
        void loadImagesAndTexturesHeaders(ifstream& stream,
            vector<ImageHeader>&,
            vector<vector<MipLevelHeader>>&,
            vector<TextureHeader>&,
            uint64_t& totalImageSize) const;

        void loadTextures(ifstream& stream);
    };

}
