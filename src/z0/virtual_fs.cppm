/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/core.hpp>
#include "z0/libraries.h"

export module z0.VirtualFS;

import z0.Constants;

import z0.Buffer;

export namespace z0 {

    /**
     * Virtual file system<br>Currently under heavy construction, do not use.
     */
    class VirtualFS {
    public:
        static constexpr string APP_URI = "app://";

        static byte* loadRGBAImage(const string& filepath, uint32_t& width, uint32_t& height, uint64_t& size, ImageFormat imageFormat);

        static void destroyImage(byte* image);

        static unique_ptr<fastgltf::GltfDataGetter> openGltf(const string& filepath);

        static ifstream openStream(const string &filepath);

        static FILE* openFile(const string& filepath);

        static vector<char> loadShader(const string &filepath);

        static vector<char> loadBinary(const string &filepath);

        static vector<char> loadBinary(const string &filepath, uint64_t size);

        static string parentPath(const string& filepath);

    private:
        static string getPath(const string& filepath);
    };

}
