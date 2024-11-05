/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ZScene;

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
        };
        struct ImageHeader {
            uint32_t format;
            uint32_t width;
            uint32_t height;
            uint32_t mipLevels;
            uint64_t dataSize;
        };
        struct MipLevelHeader {
            uint64_t offset;
            uint64_t size;
        };
    };

}
