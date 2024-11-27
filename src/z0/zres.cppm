/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.ZRes;

import z0.Texture;
import z0.Material;
import z0.Mesh;
import z0.Node;

export namespace z0 {

    /**
     * ZRes binary file format containing resources for a scene : meshes, materials, textures and images.<br>
     * It can also be used as a complete scene file since it contains a node tree.<br>
     * This file format is adapted to ZeroZero and have the following advantages :<br>
     * - Binary file format : fast loading of data without deserialization
     * - Compressed images : images are compressed in GPU-compatible format like the BCn formats to reduce the VRAM usage.<br>
     * - One big images atlas : all images are read and directly uploaded to the GPU in one pass and without using a big CPU buffer.<br>
     * - Pre calculated mip levels : all images mips levels are pre-calculated and compressed.<br>
     * - Pre calculated data : all transforms are pre-calculated.<br>
     * - Pre calculated animations data : all animation data are made relative to the original object.<br>
     *<br>
     *%A ZRes file can be created from a glTF file using the `gltf2zres` command line tool.<br>
     *<br>
     * **File format description :**
     * ```
     * Header : global header
     * array<ImageHeader + array<MipLevelInfo, mipLevels>, imagesCount> : images headers
     * array<TextureHeader, texturesCount> : textures headers
     * array<MaterialHeader, materialsCount> : materials headers
     * array<MeshHeader + array<SurfaceInfo, surfacesCount> + array<DataInfo, surfacesCount * uvsCount>, meshesCount> : meshes headers
     * array<NodeHeader + array<uint32_t, childrenCount>, nodesCount> : nodes headers
     * array<AnimationHeader + array<TrackInfo, tracksCount>, animationCount> : animation headers
     * uint32_t : indicesCount
     * array<uint32_t, indicesCount> : indices data bloc
     * uint32_t : positionsCount
     * array<vec3, positionsCount> : positions data bloc
     * uint32_t : normalsCount
     * array<vec3, normalsCount> : normals data bloc
     * uint32_t : uvsCount
     * array<vec2, uvsCount> : uvs data bloc
     * uint32_t : tangentsCount
     * array<vec4, tangentsCount> : tangents data bloc
     * array<float, keysCount> + array<vec3, keyCount> for each track, for each animation : animations data
     * array<BCn compressed image, imagesCount> : images data bloc
     *  ```
     */
    class ZRes {
    public:
        /*
         * Maximum size of strings in files
         */
        static constexpr auto NAME_SIZE{64};

        /*
         * Magic header thing
         */
        static constexpr char MAGIC[]{ 'Z', 'R', 'E', 'S' };

        /*
         * Current format version
         */
        static constexpr uint32_t VERSION{1};

        /*
         * Global file header
         */
        struct Header {
            //! Magic header thing
            char     magic[4];
            //! Format version
            uint32_t version{0};
            //! Total number of images
            uint32_t imagesCount{0};
            //! Total number of textures
            uint32_t texturesCount{0};
            //! Total number of materials
            uint32_t materialsCount{0};
            //! Total number of meshes
            uint32_t meshesCount{0};
            //! Total number of scene nodes
            uint32_t nodesCount{0};
            uint32_t animationsCount{0};
            //! Size in bytes of all the headers
            uint64_t headersSize;
        };

        /*
         * Description of an image
         */
        struct ImageHeader {
            //! Image name, copied from the original file name
            char     name[NAME_SIZE];
            //! Image format, [VkFormat format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFormat.html)
            uint32_t format;
            //! Width in pixels
            uint32_t width;
            //! Height in pixels
            uint32_t height;
            //! Number of mips levels, and number of MipLevelInfo elements in the array following this struct
            uint32_t mipLevels;
            //! Start of the image, relative to the start of the images data block
            uint64_t dataOffset;
            //! Size in bytes of all levels
            uint64_t dataSize;
        };

        /*
         * Description of a mip level for an image
         */
        struct MipLevelInfo {
            //! Start of the level, relative to the image dataOffset
            uint64_t offset;
            //! Size in bytes of the level
            uint64_t size;
        };

        /*
         * Description of a texture
         */
        struct TextureHeader {
            //! Associated image, -1 for a texture without image
            int32_t  imageIndex{-1};
            //! Minification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32_t minFilter;
            //! Magnification filter to apply to texture lookups, [VkFilter format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkFilter.html)
            uint32_t magFilter;
            //! Behavior of sampling with texture coordinates outside an image for U coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32_t samplerAddressModeU;
            //! Behavior of sampling with texture coordinates outside an image for V coordinates, [VkSamplerAddressMode format](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSamplerAddressMode.html)
            uint32_t samplerAddressModeV;
        };

        /*
         * Description of a texture attached to a material
         */
        struct TextureInfo {
            //! Attached texture, -1 if no texture
            int32_t  textureIndex{-1};
            //! Index of UV coordinates, in the UV DataInfo array of the attached surface
            uint32_t uvsIndex;
            //! UV coordinates transform
            mat3     transform;
        };

        /*
         * Description of a material
         */
        struct MaterialHeader {
            //! Material name
            char         name[NAME_SIZE];
            //! Culling mode CullMode format
            uint32_t     cullMode;
            //! Transparency, Transparency format
            uint32_t     transparency;
            //! Alpha scissor for transparency
            float        alphaScissor;
            //! Albedo color
            vec4         albedoColor;
            //! Optional albedo texture
            TextureInfo  albedoTexture;
            //! Metallic factor
            float        metallicFactor;
            //! Optional metallic texture
            TextureInfo  metallicTexture;
            //! Roughness factor
            float        roughnessFactor;
            //! Optional roughness texture
            TextureInfo  roughnessTexture;
            //! Emissive factor
            vec3         emissiveFactor;
            //! Emissive streng
            float        emissiveStrength;
            //! Optional emissive texture
            TextureInfo  emissiveTexture;
            //! Optional normal texture
            TextureInfo  normalTexture;
            //! Normal scale
            float        normalScale;
        };

        /*
         * Description of a data bloc (indices, positions, normals, tangents, uv coords)
         */
        struct DataInfo {
            //! First index in the array
            uint32_t first;
            //! Number of element
            uint32_t count;
        };

        /*
         * Description of a mesh primitive
         */
        struct SurfaceInfo {
            //! Attached material, -1 if no material
            int32_t  materialIndex{-1};
            //! Indices array
            DataInfo indices;
            //! Vertices positions array
            DataInfo positions;
            //! Normals array
            DataInfo normals;
            //! Tangents array
            DataInfo tangents;
            //! Number of DataInfo elements in the UV coordinates array of array following this struct
            uint32_t uvsCount;
        };

        /*
         * Description of a mesh
         */
        struct MeshHeader {
            //! Name
            char     name[NAME_SIZE];
            //! Number of SurfaceInfo elements in the array following this struct
            uint32_t surfacesCount;
        };

        /*
         * Description of a node in the scene
         */
        struct NodeHeader {
            //! Name
            char     name[NAME_SIZE];
            //! Associated mesh, -1 of no mesh. Nodes with mesh will be instantiated as z0::MeshInstance, other nodes as z0::Node
            int32_t  meshIndex;
            //! World transform
            mat4     transform;
            //! Number of children nodes, also the number of elements in the uint32_t array following this struct
            uint32_t childrenCount;
        };

        struct AnimationHeader {
            char     name[NAME_SIZE];
            uint32_t tracksCount;
        };

        struct TrackInfo {
            int32_t  nodeIndex{-1};
            uint32_t type;
            uint32_t interpolation;
            uint32_t keysCount;
            // + keyCount * float keyTime
            // + keyCount * variant<vec3, quat> keyValue
        };

        /**
         * Load a scene from a ZScene file and return the root node
         */
        [[nodiscard]] static shared_ptr<Node> load(const string &filename);

        /**
         * Load a scene from a ZScene data stream and return the root node
         */
        [[nodiscard]] static shared_ptr<Node> load(ifstream &stream);

        ZRes() = default;

        static void print(const Header& header);
        static void print(const ImageHeader& header);
        static void print(const MipLevelInfo& header);
        static void print(const TextureHeader& header);
        static void print(const MaterialHeader& header);
        static void print(const MeshHeader& header);
        static void print(const SurfaceInfo& header);
        static void print(const DataInfo& header);

    protected:
        Header header{};
        vector<shared_ptr<Texture>>  textures{};

        shared_ptr<Node> loadScene(ifstream& stream);

        void loadImagesAndTextures(ifstream& stream,
            const vector<ImageHeader>&,
            const vector<vector<MipLevelInfo>>&,
            const vector<TextureHeader>&,
            uint64_t totalImageSize);
    };

}
