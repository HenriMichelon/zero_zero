module;
#include <fastgltf/core.hpp>
#include "z0/libraries.h"

export module z0:VirtualFS;

import :Constants;

export namespace z0 {

    /**
     * Virtual files system
     */
    class VirtualFS {
    public:
        static constexpr string APP_URI = "app://";

        static unsigned char* loadImage(const string& path, uint32_t& width, uint32_t& height, uint64_t& size, ImageFormat imageFormat);

        static void destroyImage(unsigned char* image);

        // static unique_ptr<fastgltf::GltfDataGetter> loadGlTF(const string& filepath);
    };

}
