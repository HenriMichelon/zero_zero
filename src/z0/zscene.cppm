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

import z0.Image;

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
            uint64_t headersSize;
            uint32_t imagesCount{0};
        };

        struct ImageHeader {
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

        [[nodiscard]] static shared_ptr<ZScene> load(const string &filename);
        [[nodiscard]] static shared_ptr<ZScene> load(ifstream &stream);

        inline const vector<shared_ptr<Image>>& getImages() const { return images; };

        ZScene() = default;

    protected:
        Header                    header{};
        vector<shared_ptr<Image>> images{};

        void loadImages(ifstream& stream);
        void loadHeader(ifstream& stream);
        void loadImagesHeaders(ifstream& stream, vector<ImageHeader>&, vector<vector<MipLevelHeader>>&, uint64_t& totalImageSize) const;
    };

}
