module;
#include <stb_image.h>
#include <stb_truetype.h>
#include "z0/libraries.h"

module z0;

import :Tools;
import :Device;
import :Image;
import :Font;

namespace z0 {

    void Font::getSize(const string &text, float &width, float &height) {
        width                  = 0;
        uint32_t max_height    = 0;
        uint32_t max_descender = 0;
        for (const auto wc : text) {
            auto &cchar = getFromCache(wc);
            width += static_cast<float>(cchar.advance);
            auto descender = cchar.height - cchar.yBearing;
            max_height     = std::max(max_height, cchar.height);
            max_descender  = std::max(max_descender, descender);
        }
        height = static_cast<float>(max_height + max_descender);
    }

    vector<uint32_t> Font::renderToBitmap(const string &text, float &wwidth, float &hheight) {
        uint32_t width         = 0;
        uint32_t max_height    = 0;
        uint32_t max_descender = 0;
        for (const auto wc : text) {
            auto &cchar = getFromCache(wc);
            width += cchar.advance + cchar.xBearing;
            const auto descender = cchar.height - cchar.yBearing;
            max_height           = std::max(max_height, cchar.height);
            max_descender        = std::max(max_descender, descender);
        }
        uint32_t height = max_height + max_descender;

        auto    bitmap = vector<uint32_t>(width * height, 0);
        int32_t x      = 0;
        for (const auto wc : text) {
            auto &     cchar  = getFromCache(wc);
            const auto offset = height - cchar.yBearing - max_descender;
            for (int line = 0; line < cchar.height; line++) {
                const auto dest = bitmap.data() + (x + cchar.xBearing) + ((line + offset) * width);
                const auto src  = cchar.bitmap->data() + (line * cchar.width);
                for (int col = 0; col < cchar.width; col++) {
                    const auto value = src[col];
                    if (value != 0) { dest[col] = value; }
                }
            }
            x += cchar.advance;
        }
        wwidth  = static_cast<float>(width);
        hheight = static_cast<float>(height);
        return bitmap;
    }

    shared_ptr<Image> Font::renderToImage(const Device &device, const string &text) {
        float width, height;
        auto  bitmap = renderToBitmap(text, width, height);
        /*  auto name = str;
         name.append(".png");
         stbi_write_png(name.c_str(), width, height, STBI_rgb_alpha, bitmap.data(), width * STBI_rgb_alpha); */
        return make_shared<Image>(device,
                                  text,
                                  width,
                                  height,
                                  static_cast<int>(width * height) * STBI_rgb_alpha,
                                  bitmap.data(),
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                                  VK_FILTER_LINEAR,
                                  // don't repeat texture
                                  false);
    }

    Font::CachedCharacter &Font::getFromCache(const char c) {
        if (cache.contains(c)) {
            return cache[c];
        } else {
            auto &cchar = cache[c];
            render(cchar, c);
            return cchar;
        }
    }

    uint32_t Font::scaleFontSize(const uint32_t baseFontSize) const {
        constexpr int baseWidth               = 1920;
        constexpr int baseHeight              = 1080;
        const auto    newHeight               = Application::get().getWindow().getHeight();
        const auto    newWidth                = Application::get().getWindow().getWidth();
        const auto    horizontalScalingFactor = static_cast<float>(newWidth) / baseWidth;
        const auto    verticalScalingFactor   = static_cast<float>(newHeight) / baseHeight;
        const auto    averageScalingFactor    = (horizontalScalingFactor + verticalScalingFactor) / 2.0;
        return ceil(static_cast<uint32_t>(baseFontSize * averageScalingFactor));
    }


#ifdef __STB_INCLUDE_STB_TRUETYPE_H__

    Font::Font(const string &_name, const uint32_t _size) :
        Resource{_name},
        path{_name},
        size{_size} {
        ifstream fontFile(path.c_str(), ios::binary);
        if (!fontFile) { die("Failed to open font file", path); }
        fontBuffer = make_unique<vector<unsigned char>>((istreambuf_iterator<char>(fontFile)),
                                                        istreambuf_iterator<char>());
        if (!stbtt_InitFont(&font, fontBuffer->data(), stbtt_GetFontOffsetForIndex(fontBuffer->data(), 0))) {
            die("Failed to initialize font", path);
        }
        scale = stbtt_ScaleForPixelHeight(&font, scaleFontSize(size));
        stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
        height = ceilf((ascent - descent) * scale);
        //log(to_string(size), "->", to_string(scaleFontSize(size)), "=", to_string(height));
        ascent  = ascent * scale;
        descent = descent * scale;
    }

    /*  void savePPM(const char* filename, const unsigned char* bitmap, int width, int height) {
         std::ofstream ofs(filename, std::ios::binary);
         ofs << "P6\n" << width << " " << height << "\n255\n";
         for (int y = 0; y < height; ++y) {
             for (int x = 0; x < width; ++x) {
                 unsigned char pixel = bitmap[y * width + x];
                 ofs << pixel << pixel << pixel; // Writing RGB components (all the same for grayscale)
             }
         }
         ofs.close();
     } */

    void Font::render(CachedCharacter &cachedCharacter, const char c) const {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        cachedCharacter.advance  = advanceWidth * scale;
        cachedCharacter.xBearing = leftSideBearing * scale;

        int        width, height;
        const auto srcBitmap     = stbtt_GetCodepointBitmap(&font, 0, scale, c, &width, &height, 0, 0);
        cachedCharacter.width    = width;
        cachedCharacter.height   = this->height;
        cachedCharacter.yBearing = cachedCharacter.height;
        cachedCharacter.bitmap   = make_unique<vector<uint32_t>>(cachedCharacter.width * cachedCharacter.height, 0);

        int x1, y1, x2, y2;
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x1, &y1, &x2, &y2);
        const auto yOffset = (ascent + y1);

        const auto dstBitmap = cachedCharacter.bitmap->data();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t gray = srcBitmap[y * width + x];
                if (gray != 0) {
                    dstBitmap[(y + yOffset) * cachedCharacter.width + x] = (gray << 24) | (gray << 16) | (gray << 8)
                            |
                            gray;
                };
            }
        }
        /* string name = "f_";
        name += c;
        name += ".ppm";
        savePPM( name.c_str(), srcBitmap, width, height); */
        stbtt_FreeBitmap(srcBitmap, nullptr);
    }

    Font::~Font() {
    }

#endif

}
