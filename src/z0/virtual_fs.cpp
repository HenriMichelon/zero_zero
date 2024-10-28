module;
#include <cassert>
#include <stb_image.h>
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


    unsigned char* VirtualFS::loadImage(const string& path,
                                        uint32_t& width, uint32_t& height, uint64_t& size,
                                        const ImageFormat imageFormat) {
        assert(imageFormat == IMAGE_R8G8B8A8);
        filesystem::path filename;
        if (path.starts_with(APP_URI)) {
            filename = Application::get().getConfig().appDir;
        } else {
            die("Unknown resource type");
        }
        filename /= path.substr(APP_URI.size());

        std::ifstream file(filename, std::ios::binary);
        if (!file) die("Error: Could not open file ", filename.string());

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
        return imageData;
    }

    void VirtualFS::destroyImage(unsigned char* image) {
        stbi_image_free(image);
    }

    // class IStreamDataGetter : public fastgltf::GltfDataGetter {
    // public:
    //     explicit IStreamDataGetter(std::istream& stream)
    //         : stream_(stream) {}
    //
    //     void read(void* ptr, std::size_t count) override {
    //
    //     }
    //
    //     span<std::byte> read(std::size_t count, std::size_t padding) override {
    //
    //     }
    //
    //     void reset() {
    //     }
    //
    //     std::size_t bytesRead() {
    //
    //     }
    //
    //     virtual std::size_t totalSize() {
    //
    //     }
    //
    //     std::optional<std::vector<std::byte>> getData(fastgltf::GltfDataLocation location, std::size_t offset, std::size_t size) override {
    //         if (!stream_) {
    //             return std::nullopt;
    //         }
    //
    //         // Seek to the specified offset in the stream
    //         stream_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    //         if (!stream_) {
    //             return std::nullopt;
    //         }
    //
    //         // Read `size` bytes from the stream
    //         std::vector<std::byte> data(size);
    //         stream_.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));
    //
    //         // Check if the read was successful
    //         if (!stream_) {
    //             return std::nullopt;
    //         }
    //
    //         return data;
    //     }
    //
    // private:
    //     std::istream& stream_;
    // };
    //
    // unique_ptr<fastgltf::GltfDataGetter> VirtualFS::loadGlTF(const string& filepath) {
    //
    // }

}
