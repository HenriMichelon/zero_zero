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
import z0.Material;

export namespace z0 {

    /**
     * ZScene file format
     */
    class ZScene {
    public:
        static constexpr auto NAME_SIZE{64};

        struct Header {
            static constexpr char MAGIC[]{ 'Z', 'S', 'C', 'N' };
            static constexpr uint32_t VERSION{1};
            char     magic[4];
            uint32_t version{0};
            uint32_t imagesCount{0};
            uint32_t texturesCount{0};
            uint32_t materialsCount{0};
            uint64_t headersSize;
        };

        struct ImageHeader {
            char     name[NAME_SIZE];
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
            int32_t  imageIndex;
            uint32_t minFilter;
            uint32_t magFilter;
            uint32_t samplerAddressModeU;
            uint32_t samplerAddressModeV;
        };

        struct TextureInfo {
            int32_t   textureIndex;
            mat3      transform;
        };

        struct MaterialHeader {
            char         name[NAME_SIZE];
            uint32_t     cullMode;
            uint32_t     transparency;
            float        alphaScissor;
            vec4         albedoColor;
            TextureInfo  albedoTexture;
            float        metallicFactor;
            TextureInfo  metallicTexture;
            float        roughnessFactor;
            TextureInfo  roughnessTexture;
            vec3         emissiveFactor;
            float        emissiveStrength;
            TextureInfo  emissiveTexture;
            TextureInfo  normalTexture;
            float        normalScale;
        };

        [[nodiscard]] static shared_ptr<ZScene> load(const string &filename);
        [[nodiscard]] static shared_ptr<ZScene> load(ifstream &stream);

        inline const vector<shared_ptr<Texture>>& getTextures() const { return textures; };
        inline const vector<shared_ptr<Material>>& getMaterials() const { return materials; };

        ZScene() = default;

        static void print(const Header& header);
        static void print(const ImageHeader& header);
        static void print(const MipLevelHeader& header);
        static void print(const TextureHeader& header);
        static void print(const MaterialHeader& header);

    protected:
        Header header{};
        vector<shared_ptr<Texture>> textures{};
        vector<shared_ptr<Material>> materials{};

        void loadScene(ifstream& stream);

        void loadImagesAndTextures(ifstream& stream,
            const vector<ImageHeader>&,
            const vector<vector<MipLevelHeader>>&,
            const vector<TextureHeader>&,
            uint64_t totalImageSize);
    };

}
