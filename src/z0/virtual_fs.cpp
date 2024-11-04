/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include <stb_image.h>
#include <fastgltf/core.hpp>
#include "z0/libraries.h"

module z0;

import :Tools;
import :Constants;
import :Application;
import :VirtualFS;

namespace z0 {

    struct StreamWrapper {
        std::ifstream* stream;
    };

    int readCallback(void* user, char* data, const int size) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        wrapper->stream->read(data, size);
        return static_cast<int>(wrapper->stream->gcount());
    }

    void skipCallback(void* user, const int n) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        wrapper->stream->seekg(n, std::ios::cur);
    }

    int eofCallback(void* user) {
        const auto* wrapper = static_cast<StreamWrapper*>(user);
        return wrapper->stream->eof() ? 1 : 0;
    }

    constexpr auto callbacks = stbi_io_callbacks{
        .read = readCallback,
        .skip = skipCallback,
        .eof  = eofCallback,
    };

    byte* VirtualFS::loadImage(const string& filepath,
                                uint32_t& width, uint32_t& height, uint64_t& size,
                                const ImageFormat imageFormat) {
        assert(imageFormat == IMAGE_R8G8B8A8_SRGB || imageFormat == IMAGE_R8G8B8A8_UNORM);
        ifstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) die("Error: Could not open file ", filepath);

        StreamWrapper wrapper{&file};
        int texWidth, texHeight, texChannels;
        unsigned char* imageData = stbi_load_from_callbacks(
            &callbacks, &wrapper,
            &texWidth, &texHeight,
            &texChannels, STBI_rgb_alpha);
        if (!imageData) die("Error: ", string{stbi_failure_reason()});
        width = static_cast<uint32_t>(texWidth);
        height = static_cast<uint32_t>(texHeight);
        size = width * height * STBI_rgb_alpha;
        return reinterpret_cast<byte*>(imageData);
    }

    void VirtualFS::destroyImage(byte* image) {
        stbi_image_free(image);
    }

    class GltfFileStream : public fastgltf::GltfDataGetter {
    public:
        explicit GltfFileStream(const std::string& filename) : stream{filename, std::ios::binary | std::ios::ate} {
            if (!stream.is_open()) die("Error: Could not open file ", filename);
            fileSize = stream.tellg();
            stream.seekg(0, std::ios::beg);
            bytesReadCount = 0;
        }

        ~GltfFileStream() noexcept override = default;

        void read(void* ptr, const size_t count) override {
            stream.read(static_cast<char*>(ptr), count);
            bytesReadCount += count;
        }

        fastgltf::span<byte> read(const size_t count, const size_t padding) override {
            buffer.resize(count + padding);
            stream.read(reinterpret_cast<char*>(buffer.data()), count);
            bytesReadCount += count;
            return fastgltf::span<byte>(buffer.data(), count + padding);
        }

        void reset() override {
            stream.clear();
            stream.seekg(0, std::ios::beg);
            bytesReadCount = 0;
        }

        size_t bytesRead() override {
            return bytesReadCount;
        }

        size_t totalSize() override {
            return fileSize;
        }

    private:
        ifstream     stream;
        size_t       fileSize;
        size_t       bytesReadCount;
        vector<byte> buffer;
    };

    unique_ptr<fastgltf::GltfDataGetter> VirtualFS::openGltf(const string& filepath) {
        return make_unique<GltfFileStream>(getPath(filepath));
    }

    ifstream VirtualFS::openFile(const string& filepath) {
        ifstream file(getPath(filepath), std::ios::binary);
        if (!file.is_open()) die("Error: Could not open file ", filepath);
        return file;
    }

    vector<char> VirtualFS::loadShader(const string &filepath) {
        auto path = APP_URI + "shaders/" + filepath + ".spv";
        ifstream file(getPath(path), std::ios::ate | std::ios::binary);
        if (!file.is_open()) die("failed to open file : ", filepath);
        const size_t fileSize = file.tellg();
        vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }

    string VirtualFS::parentPath(const string& filepath) {
        const auto lastSlash = filepath.find_last_of("/");
        if (lastSlash == std::string::npos) return "";
        return filepath.substr(0, lastSlash+1);
    }

    string VirtualFS::getPath(const string& filepath) {
        string filename;
        if (filepath.starts_with(APP_URI)) {
            filename = Application::get().getConfig().appDir.string();
        } else {
            die("Unknown resource type ", filepath);
        }
        return (filename + "/" + filepath.substr(APP_URI.size()));
    }

}
