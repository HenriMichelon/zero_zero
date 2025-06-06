/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_truetype.h>
#include "z0/libraries.h"

module z0.resources.Font;

import z0.Application;
import z0.Log;
import z0.Tools;
import z0.VirtualFS;

import z0.resources.Image;

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
        const uint32_t height = max_height + max_descender;

        auto    bitmap = vector<uint32_t>(width * height, 0);
        int32_t x      = 0;
        for (const auto wc : text) {
            const auto &cchar  = getFromCache(wc);
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

    Font::CachedCharacter &Font::getFromCache(const char c) {
        if (characterCache.contains(c)) {
            return characterCache[c];
        } else {
            auto &cchar = characterCache[c];
            render(cchar, c);
            return cchar;
        }
    }

    uint32_t Font::scaleFontSize(const uint32_t baseFontSize) const {
        constexpr int baseWidth  = 1920;
        constexpr int baseHeight = 1080;
        const auto    newHeight = app().getWindow().getHeight();
        const auto    newWidth  = app().getWindow().getWidth();
        const auto    horizontalScalingFactor= static_cast<float>(newWidth) / baseWidth;
        const auto    verticalScalingFactor  = static_cast<float>(newHeight) / baseHeight;
        const auto    averageScalingFactor   = (horizontalScalingFactor + verticalScalingFactor) / 2.0f;
        return static_cast<uint32_t>(ceil(static_cast<uint32_t>(baseFontSize * averageScalingFactor)));
    }

    Font::Font(const Font &font, const uint32_t size) : Font{font.getFontName(), size} {}


#ifdef __STB_INCLUDE_STB_TRUETYPE_H__

    Font::Font(const string &name, const uint32_t size) :
        Resource{name},
        path{name},
        size{size} {
        ifstream fontFile = VirtualFS::openReadStream(path);
        fontBuffer = make_unique<vector<unsigned char>>((istreambuf_iterator(fontFile)),
                                                        istreambuf_iterator<char>());
        if (!stbtt_InitFont(&font, fontBuffer->data(), stbtt_GetFontOffsetForIndex(fontBuffer->data(), 0))) {
            die("Failed to initialize font", path);
        }
        scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(scaleFontSize(size)));
        stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
        height = static_cast<int>(ceilf((ascent - descent) * scale));
        DEBUG("Font size : ", size, "->", scaleFontSize(size), "=", height);
        ascent  = static_cast<int>(ascent * scale);
        descent = static_cast<int>(descent * scale);
    }

    void Font::render(CachedCharacter &cachedCharacter, const char c) const {
        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, c, &advanceWidth, &leftSideBearing);
        cachedCharacter.advance  = static_cast<int32_t>(advanceWidth * scale);
        cachedCharacter.xBearing = static_cast<int32_t>(leftSideBearing * scale);

        int width, height;
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
                const uint8_t gray = srcBitmap[y * width + x];
                if (gray != 0) {
                    dstBitmap[(y + yOffset) * cachedCharacter.width + x] = (gray << 24) | (gray << 16) | (gray << 8)
                            |
                            gray;
                };
            }
        }
        stbtt_FreeBitmap(srcBitmap, nullptr);
    }

    Font::~Font() {
    }

#endif

}
