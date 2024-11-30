/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
//#include <ktx.h>
#include <ddspp.h>
#include <dxgiformat.h>
#include <volk.h>
#include "z0/libraries.h"

module z0.Image;

import z0.Constants;
import z0.Tools;
import z0.VirtualFS;

import z0.Device;
import z0.VulkanImage;

namespace z0 {

    shared_ptr<Image> Image::create(
                const Device& device,
                uint32_t width, uint32_t height,
                uint64_t imageSize, const void *data,
                const string & name, const ImageFormat format) {
        return make_shared<VulkanImage>(
            device,
            name, width, height, imageSize, data,
            format == ImageFormat::R8G8B8A8_SRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM);
    }

    unordered_map<DXGI_FORMAT, VkFormat> dxgiToVulkanFormat = {
        // Undefined format
        { DXGI_FORMAT_UNKNOWN, VK_FORMAT_UNDEFINED },

        // 32-bit floating-point formats
        { DXGI_FORMAT_R32G32B32A32_FLOAT, VK_FORMAT_R32G32B32A32_SFLOAT },
        { DXGI_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT },
        { DXGI_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SINT },
        { DXGI_FORMAT_R32G32B32_FLOAT, VK_FORMAT_R32G32B32_SFLOAT },
        { DXGI_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_UINT },
        { DXGI_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SINT },

        // 16-bit floating-point formats
        { DXGI_FORMAT_R16G16B16A16_FLOAT, VK_FORMAT_R16G16B16A16_SFLOAT },
        { DXGI_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_UNORM },
        { DXGI_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT },
        { DXGI_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SNORM },
        { DXGI_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT },
        { DXGI_FORMAT_R32G32_FLOAT, VK_FORMAT_R32G32_SFLOAT },
        { DXGI_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_UINT },
        { DXGI_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SINT },

        // 32-bit and 24-bit depth-stencil formats
        { DXGI_FORMAT_D32_FLOAT, VK_FORMAT_D32_SFLOAT },
        { DXGI_FORMAT_D32_FLOAT_S8X24_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT },
        { DXGI_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },

        // 10-bit color formats
        { DXGI_FORMAT_R10G10B10A2_UNORM, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
        { DXGI_FORMAT_R10G10B10A2_UINT, VK_FORMAT_A2B10G10R10_UINT_PACK32 },
        { DXGI_FORMAT_R11G11B10_FLOAT, VK_FORMAT_B10G11R11_UFLOAT_PACK32 },

        // 8-bit and 16-bit formats
        { DXGI_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },
        { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, VK_FORMAT_R8G8B8A8_SRGB },
        { DXGI_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UINT },
        { DXGI_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM },
        { DXGI_FORMAT_R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SINT },
        { DXGI_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM },
        { DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, VK_FORMAT_B8G8R8A8_SRGB },
        { DXGI_FORMAT_R16G16_FLOAT, VK_FORMAT_R16G16_SFLOAT },
        { DXGI_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_UNORM },
        { DXGI_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_UINT },
        { DXGI_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_SNORM },
        { DXGI_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SINT },
        { DXGI_FORMAT_R32_FLOAT, VK_FORMAT_R32_SFLOAT },
        { DXGI_FORMAT_R32_UINT, VK_FORMAT_R32_UINT },
        { DXGI_FORMAT_R32_SINT, VK_FORMAT_R32_SINT },
        { DXGI_FORMAT_R8_UNORM, VK_FORMAT_R8_UNORM },
        { DXGI_FORMAT_R8_UINT, VK_FORMAT_R8_UINT },
        { DXGI_FORMAT_R8_SNORM, VK_FORMAT_R8_SNORM },
        { DXGI_FORMAT_R8_SINT, VK_FORMAT_R8_SINT },

        // Block-compressed formats (BC1 - BC7)
        { DXGI_FORMAT_BC1_UNORM, VK_FORMAT_BC1_RGBA_UNORM_BLOCK },
        { DXGI_FORMAT_BC1_UNORM_SRGB, VK_FORMAT_BC1_RGBA_SRGB_BLOCK },
        { DXGI_FORMAT_BC2_UNORM, VK_FORMAT_BC2_UNORM_BLOCK },
        { DXGI_FORMAT_BC2_UNORM_SRGB, VK_FORMAT_BC2_SRGB_BLOCK },
        { DXGI_FORMAT_BC3_UNORM, VK_FORMAT_BC3_UNORM_BLOCK },
        { DXGI_FORMAT_BC3_UNORM_SRGB, VK_FORMAT_BC3_SRGB_BLOCK },
        { DXGI_FORMAT_BC4_UNORM, VK_FORMAT_BC4_UNORM_BLOCK },
        { DXGI_FORMAT_BC4_SNORM, VK_FORMAT_BC4_SNORM_BLOCK },
        { DXGI_FORMAT_BC5_UNORM, VK_FORMAT_BC5_UNORM_BLOCK },
        { DXGI_FORMAT_BC5_SNORM, VK_FORMAT_BC5_SNORM_BLOCK },
        { DXGI_FORMAT_BC6H_UF16, VK_FORMAT_BC6H_UFLOAT_BLOCK },
        { DXGI_FORMAT_BC6H_SF16, VK_FORMAT_BC6H_SFLOAT_BLOCK },
        { DXGI_FORMAT_BC7_UNORM, VK_FORMAT_BC7_UNORM_BLOCK },
        { DXGI_FORMAT_BC7_UNORM_SRGB, VK_FORMAT_BC7_SRGB_BLOCK },
    };

    Image::Image(
        const uint32_t width,
        const uint32_t height,
        const string & name):
        Resource{name}, width{width}, height{height} {};

    shared_ptr<Image> Image::createBlankImage(const Device& device) {
        const auto& blankJPEG = createBlankJPG();
        return create(device, 1, 1, blankJPEG.size(), blankJPEG.data(), "Blank");
    }

    shared_ptr<Image> Image::load(const string &filepath, const ImageFormat imageFormat) {
        shared_ptr<Image> result;
        auto &device = Device::get();
        if (filepath.ends_with(".dds")) {
            auto ddsData = VirtualFS::loadBinary(filepath);
            ddspp::Descriptor desc;
            if(ddspp::Success != decode_header(reinterpret_cast<unsigned char*>(ddsData.data()), desc)) {
                die("Failed to decode DDS header for", filepath);
            }
            const auto format = dxgiToVulkanFormat.at(static_cast<DXGI_FORMAT>(desc.format));
            result = make_shared<VulkanImage>(
                    device,
                    filepath, desc.width, desc.height,
                    ddsData.size() - desc.headerSize, ddsData.data() + desc.headerSize,
                    imageFormat == ImageFormat::R8G8B8A8_SRGB ? VulkanImage::formatSRGB(format, filepath): format,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, true);
        }
        // if (filepath.ends_with(".ktx2")) {
        //     const auto ktxFile = VirtualFS::openFile(filepath);
        //     ktxTexture2* texture;
        //     if (KTX_SUCCESS != ktxTexture2_CreateFromStdioStream(ktxFile,
        //         KTX_TEXTURE_CREATE_NO_FLAGS,
        //         &texture)) {
        //         die("Failed to create KTX texture from file stream");
        //     }
        //     if (ktxTexture2_NeedsTranscoding(texture)) {
        //         const ktx_transcode_fmt_e transcodeFormat =
        //                       Device::get().isFormatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK) ? KTX_TTF_ASTC_4x4_RGBA :
        //                       Device::get().isFormatSupported(VK_FORMAT_BC7_SRGB_BLOCK) ? KTX_TTF_BC7_RGBA :
        //                       Device::get().isFormatSupported(VK_FORMAT_BC3_SRGB_BLOCK) ? KTX_TTF_BC3_RGBA :
        //                       Device::get().isFormatSupported(VK_FORMAT_BC1_RGBA_SRGB_BLOCK) ? KTX_TTF_BC1_OR_3 :
        //                       KTX_TTF_RGBA32;
        //         if (KTX_SUCCESS != ktxTexture2_TranscodeBasis(texture, transcodeFormat, 0)) {
        //             die("Failed to transcode KTX2 to BC/ASTC");
        //         }
        //     }
        //     auto image = make_shared<KTXVulkanImage>(
        //             Device::get(), filepath,
        //             texture,
        //             VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        //             VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
        //             imageFormat == IMAGE_R8G8B8A8_SRGB);
        //     ktxTexture_Destroy((ktxTexture*)texture);
        //     fclose(ktxFile);
        //     return image;
        // }
        else {
            uint32_t texWidth, texHeight;
            uint64_t imageSize;
            auto *pixels = VirtualFS::loadRGBAImage(filepath, texWidth, texHeight, imageSize, imageFormat);
            if (!pixels) { die("failed to load texture image!"); }
            result = create(device, texWidth, texHeight, imageSize, pixels, filepath);
            VirtualFS::destroyImage(pixels);
        }
        return result;
    }

}
