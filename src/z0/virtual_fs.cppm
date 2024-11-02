/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/core.hpp>
#include "z0/libraries.h"

export module z0:VirtualFS;

import :Constants;

export namespace z0 {

    /**
     * Virtual file system
     */
    class VirtualFS {
    public:
        static constexpr string APP_URI = "app://";

        static byte* loadImage(const string& filepath, uint32_t& width, uint32_t& height, uint64_t& size, ImageFormat imageFormat);

        static void destroyImage(byte* image);

        static unique_ptr<fastgltf::GltfDataGetter> openGltf(const string& filepath);

        static ifstream openFile(const string& filepath);

        static vector<char> loadShader(const string &filepath);

        static string parentPath(const string& filepath);

    private:
        static string getPath(const string& filepath);
    };

}
