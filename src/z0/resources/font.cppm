module;
#include "stb_truetype.h"
#include "z0/libraries.h"

export module z0:Font;

import :Tools;
import :Resource;
import :Image;
import :Device;
import :Application;

export namespace z0 {

    /**
     * A font resource to render text in bitmaps.
     * A font is a combination of a font file name and a size.
     * Supports true type font files (cf https://github.com/nothings/stb/blob/master/stb_truetype.h).
     * The font size is automatically scaled based on the resolution, from a base resolution of 1920x1080
     * (14 is 14 pixels height in this resolution)
     */
    class Font : public Resource {
    public:
        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         * @param size : height in pixels on a base resolution of 1920x1080
         */
        explicit Font(const string &path, uint32_t size);

        ~Font() override;

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const string &text, float &width, float &height) {
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

        /**
         *  Renders a string into a RGBA bitmap (stored in CPU memory).
         *  Glyphs are white with alpha channel mapped to the glyphs geometry
         *   @param text : text to render
         *   @param wwidth : width of the resulting bitmap
         *   @param hheight : height of the resulting bitmap
         *   @return 32 bits RGBA bitmap stored in CPU memory
        */
        [[nodiscard]] vector<uint32_t> renderToBitmap(const string &text, float &wwidth, float &hheight) {
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

        /**
         * Renders a string into a RGBA Image resource (stored in GPU memory).
         * Glyphs are white with alpha channel mapped to the glyphs geometry
         * @param device : GPU where the Image will be stored
         * @param text : text to render
         * @result 32 bits RGBA bitmap stored in GPU memory with a VK_FORMAT_R8G8B8A8_SRGB format, clamped to border and VK_IMAGE_TILING_OPTIMAL tiling
         */
        [[nodiscard]] shared_ptr<Image> renderToImage(const Device &device, const string &text) {
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
                                      // don't repeat texture
                                      false);
        }

        /**
         * Returns the font path. Useful to create another Font resource with a different size
         */
        [[nodiscard]] const string &getFontName() const { return path; }

        /**
         * Returns the font height in pixels (NOT scaled size, but the size given to the Font constructor)
         */
        [[nodiscard]] uint32_t getFontSize() const { return size; }

    private:
        // Already rendered characters
        struct CachedCharacter {
            int32_t                      advance;
            int32_t                      xBearing;
            int32_t                      yBearing;
            uint32_t                     width;
            uint32_t                     height;
            unique_ptr<vector<uint32_t>> bitmap;
        };

        map<char, CachedCharacter> cache;
        const string               path;
        const uint32_t             size;

        [[nodiscard]] CachedCharacter &getFromCache(const char c) {
            if (cache.contains(c)) {
                return cache[c];
            } else {
                auto &cchar = cache[c];
                render(cchar, c);
                return cchar;
            }
        }

        void render(CachedCharacter &, const char) const;

        [[nodiscard]] uint32_t scaleFontSize(const uint32_t baseFontSize) const {
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
        float                             scale;
        int                               ascent;
        int                               descent;
        int                               lineGap;
        int                               height;
        stbtt_fontinfo                    font;
        unique_ptr<vector<unsigned char>> fontBuffer;
#endif
    };

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__

    Font::Font(const string &_name, const uint32_t _size):
        Resource{_name}, path{_name}, size{_size} {
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
                    dstBitmap[(y + yOffset) * cachedCharacter.width + x] = (gray << 24) | (gray << 16) | (gray << 8) |
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
