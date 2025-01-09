/*
 * Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/core.hpp>
#include <volk.h>
// #include <ddspp.h>
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#include "z0/libraries.h"


module image;

import miplevel;
import converter;

// Image loading & transcoding threads
vector<thread> loadingThreads;
exception_ptr  loadingThreadsException{nullptr};
mutex          loadingThreadsExceptionMutex;

// Jobs queue to limit the number of threads
struct JobUnit {
    string            name;
    vector<MipLevel>& mipLevels;
    const void*       srcData;
    const size_t      srcDataSize;
    const string&     dstFormat;
    const bool        verbose;
};
queue<JobUnit> jobsQueue;
mutex          jobsQueueMutex;

// Used to check the image compatibility with the compression format
const map<uint32_t, uint32_t> blockSizes = {
    { VK_FORMAT_BC1_RGBA_UNORM_BLOCK,  8 },
    { VK_FORMAT_BC1_RGBA_SRGB_BLOCK,  8 },
    { VK_FORMAT_BC2_UNORM_BLOCK,  16 },
    { VK_FORMAT_BC2_SRGB_BLOCK,  16 },
    { VK_FORMAT_BC3_SRGB_BLOCK,  16 },
    { VK_FORMAT_BC4_UNORM_BLOCK,  16 },
    { VK_FORMAT_BC5_UNORM_BLOCK,  8 },
    { VK_FORMAT_BC5_SNORM_BLOCK,  8 },
    { VK_FORMAT_BC7_UNORM_BLOCK,  16 },
    { VK_FORMAT_BC7_SRGB_BLOCK,  16 },
};

// uint32_t calculatePadding(const uint64_t size, const uint32_t format) {
//     auto blockSize = blockSizes.at(format);
//     blockSize *= blockSize;
//     return (blockSize - (size % blockSize)) % blockSize;
// }

// Used to check if we need to resize an image to align the sizes (mandatory for VRAM compressed images in Vulkan)
inline bool isMultipleOfBlockSize(const uint64_t dimension, const uint32_t blockSize) {
    return (dimension % (blockSize*blockSize)) == 0;
}

// Calculate a power of two aligned (inferior) size
inline uint32_t calculateAlignedSize(const uint32_t dimension) {
    return 1u << static_cast<uint32_t>(floor(std::log2(dimension)));
}

// Generate all mipmaps for an image and create the mipmaps levels collection
void generateMipMaps(vector<MipLevel>& outMipLevels,
                     const uint32_t srcWidth,
                     const uint32_t srcHeight,
                     const uint32_t srcChannels,
                     const uint8_t* srcData) {
    auto currentWidth = srcWidth;
    auto currentHeight = srcHeight;
    auto previousData = srcData;

    // Continue generating mip levels until reaching 4x4 resolution
    while (currentWidth > 4 || currentHeight > 4) {
        const auto width = (currentWidth > 1) ? currentWidth / 2 : 1;
        const auto height = (currentHeight > 1) ? currentHeight / 2 : 1;
        const auto dataSize = width * height * srcChannels;
        const auto dataVector = make_shared<vector<uint8_t>>(static_cast<vector<uint8_t>::size_type>(dataSize));
        const auto data = dataVector->data();
        // Generate mip data by averaging 2x2 blocks from the previous level
        for (auto y = 0; y < height; ++y) {
            for (auto x = 0; x < width; ++x) {
                const auto srcX = x * 2;
                const auto srcY = y * 2;
                // Average 2x2 block
                for (int c = 0; c < srcChannels; ++c) {
                    const auto sum = previousData[(srcY * currentWidth + srcX) * srcChannels + c] +
                                   previousData[(srcY * currentWidth + (srcX + 1)) * srcChannels + c] +
                                   previousData[((srcY + 1) * currentWidth + srcX) * srcChannels + c] +
                                   previousData[((srcY + 1) * currentWidth + (srcX + 1)) * srcChannels + c];
                    data[(y * width + x) * srcChannels + c] = static_cast<uint8_t>(sum / srcChannels);
                }
            }
        }
        outMipLevels.push_back(MipLevel{ width, height, std::move(dataVector) });
        currentWidth = width;
        currentHeight = height;
        previousData = data;
    }
}

// Load the image from the raw data buffer into an RGBA uncompressed image,
// generate the mipmaps using the CPU and run the converter on each mipmap level using the GPU
void loadAndConvertImage(const string&     name,
                         vector<MipLevel>& outMipLevels,
                         const void*       srcData,
                         const size_t      srcDataSize,
                         const string&     dstFormat,
                         const bool        verbose) {
    {
        // Check if we need to stop if there is any error in any loading thread
        auto lock = lock_guard(loadingThreadsExceptionMutex);
        if (loadingThreadsException) { return; }
    }
    if (verbose) { cout << "\tProcessing image " << name << " (" << dstFormat << ")" << endl; }
    // Load the uncompressed image
    int width, height, channels;
    auto *data = stbi_load_from_memory(static_cast<stbi_uc const *>(srcData),
                                                 static_cast<int>(srcDataSize),
                                                 &width,
                                                 &height,
                                                 &channels,
                                                 STBI_rgb_alpha);
    {
        auto lock = lock_guard(loadingThreadsExceptionMutex);
        if (loadingThreadsException) { return; }
    }
    if (data) {
        // Check if we need to align the image sizes
        auto blockSize = blockSizes.at(formats.at(dstFormat).format);
        if ((!isMultipleOfBlockSize(width, blockSize) || !isMultipleOfBlockSize(height, blockSize)) &&
            (width > 4 && height > 4)) {
            blockSize *= blockSize;
            auto alignedWidth = calculateAlignedSize(width);
            auto alignedHeight = calculateAlignedSize(height);
            if (verbose) {
                cout << format("\tResizing {} to block-aligned size {}x{} -> {}x{}\n",
                    name, width, height, alignedWidth, alignedHeight); }
            auto *resizedData = stbir_resize_uint8_linear(
                data, width, height, 0,
                nullptr, alignedWidth, alignedHeight, 0,
                STBIR_RGBA);
            stbi_image_free(data);
            data = resizedData;
            width = alignedWidth;
            height = alignedHeight;
            {
                auto lock = lock_guard(loadingThreadsExceptionMutex);
                if (loadingThreadsException) { return; }
            }
        }

        // Generate the mipmaps
        auto inMipLevels = vector<MipLevel>();
        generateMipMaps(inMipLevels, width, height, STBI_rgb_alpha, data);
        {
            auto lock = lock_guard(loadingThreadsExceptionMutex);
            if (loadingThreadsException) { return; }
        }
        if (inMipLevels.empty()) {
            const auto dataSize = width * height * STBI_rgb_alpha;
            inMipLevels.push_back({
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                make_shared<vector<uint8_t>>(static_cast<vector<uint8_t>::size_type>(dataSize))});
            memcpy(inMipLevels.at(0).data->data(), data, dataSize);
        }
        stbi_image_free(data);

        // Compress all the mipmaps levels
        outMipLevels.resize(inMipLevels.size());
        const Converter converter; // We use one converter per image to allow multi-threading
        {
            auto lock = lock_guard(loadingThreadsExceptionMutex);
            if (loadingThreadsException) { return; }
        }
        for(auto level = 0; level < inMipLevels.size(); ++level) {
            converter.convert(inMipLevels.at(level),
                              outMipLevels.at(level),
                              STBI_rgb_alpha,
                              dstFormat);
            auto lock = lock_guard(loadingThreadsExceptionMutex);
            if (loadingThreadsException) { return; }
        }
        flush(cout);
    } else {
        throw "Error loading image " + name;
    }
}

void startImagesLoading(const int maxThreads) {
    // Start maxThreads threads to load & transcode the images in multi-threaded batch mode
    for (auto i = 0; i < maxThreads; i++) {
        loadingThreads.push_back(thread([i] {
            try {
                while (true) {
                    // Check if we have a conversion job to run
                    auto lock = unique_lock(jobsQueueMutex);
                    if (jobsQueue.empty()) {
                        lock.unlock();
                        return;
                    }
                    // Get the next job to run
                    auto job = jobsQueue.front();
                    jobsQueue.pop();
                    lock.unlock();
                    // Load & transcode one image
                    loadAndConvertImage(
                        job.name,
                        job.mipLevels,
                        job.srcData,
                        job.srcDataSize,
                        job.dstFormat,
                        job.verbose
                    );
                }
            } catch (...) {
                // Pass any exception throws from the loading threads to the main thread
                auto lock = lock_guard(loadingThreadsExceptionMutex);
                loadingThreadsException = current_exception();
            }
        }));
    }

    // If any thread have throw an exception, re-throw it to pass it to the main function
    ranges::for_each(loadingThreads, [&](auto& thread) { thread.join(); });
    if (loadingThreadsException) {
        rethrow_exception(loadingThreadsException);
    }
}


void equeueImageLoading(fastgltf::Asset&   asset,
                       fastgltf::Image&   image,
                       vector<MipLevel>&  outMipLevels,
                       const string&      dstFormat,
                       const bool         verbose) {
    const auto& name = image.name.data();
    visit(fastgltf::visitor{
      [](auto &) {},
      [&](fastgltf::sources::URI &) {
          throw "External textures files for glTF not supported";
      },
      [&](fastgltf::sources::Vector &vector) {
          jobsQueue.push({name, outMipLevels,
                vector.bytes.data(), vector.bytes.size(),
                dstFormat, verbose});
      },
      [&](fastgltf::sources::BufferView &view) {
          const auto &bufferView = asset.bufferViews.at(view.bufferViewIndex);
          auto &buffer           = asset.buffers.at(bufferView.bufferIndex);
          visit(fastgltf::visitor{
                [](auto &arg) {},
                [&](fastgltf::sources::Vector &vector) {
                    jobsQueue.push({name, outMipLevels,
                        vector.bytes.data() + bufferView.byteOffset, bufferView.byteLength,
                        dstFormat, verbose});
                },
                [&](fastgltf::sources::Array &array) {
                    jobsQueue.push({name, outMipLevels,
                        array.bytes.data() + bufferView.byteOffset, bufferView.byteLength,
                        dstFormat, verbose});
                },
            },
            buffer.data);
      },
      },
    image.data);
}

/*
const map<string, const ddspp::DXGIFormat> formatsDDS = {
    { "bc1",  ddspp::BC1_UNORM},
    { "bc2",  ddspp::BC2_UNORM},
    { "bc3",  ddspp::BC3_UNORM},
    { "bc4",  ddspp::BC4_UNORM},
    { "bc4s", ddspp::BC4_SNORM},
    { "bc5",  ddspp::BC5_UNORM},
    { "bc5s", ddspp::BC5_SNORM},
    { "bc7",  ddspp::BC7_UNORM},
};

void saveImageDDS(const string& filename, const vector<MipLevel>& outMipLevels, const string& dstFormat) {
    ddspp::Header header;
    ddspp::HeaderDXT10 h10;
    ddspp::encode_header(
            formatsDDS.at(dstFormat),
            outMipLevels.at(0).width,
            outMipLevels.at(0).height,
            1,
            ddspp::Texture2D,
            outMipLevels.size(),
            1,
            header,
            h10);
    auto out = ofstream(filename, ios::binary);
    out.write(reinterpret_cast<const char*>(&ddspp::DDS_MAGIC), sizeof(ddspp::DDS_MAGIC));
    out.write(reinterpret_cast<char*>(&header), sizeof(ddspp::Header));
    out.write(reinterpret_cast<char*>(&h10), sizeof(ddspp::HeaderDXT10));

    for (const auto & mipLevel : outMipLevels) {
        out.write(reinterpret_cast<char*>(mipLevel.data), mipLevel.dataSize);
    }
    out.close();
}
*/