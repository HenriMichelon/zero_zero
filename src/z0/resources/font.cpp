#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#endif

namespace z0 {

    void Font::getSize(const std::string &str, float& width, float& height) {
        width = 0;
        uint32_t max_height = 0;
        uint32_t max_descender = 0;
        for (auto wc : str) {
            auto& cchar = getFromCache(wc);
            width += static_cast<float>(cchar.advance);
            auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        height = static_cast<float>(max_height + max_descender);
    }

    Font::CachedCharacter& Font::getFromCache(char CHAR) {
        if (cache.contains(CHAR)) {
            return cache[CHAR];
        } else {
            auto& cchar = cache[CHAR];
            render(cchar, CHAR);
            return cchar;
        }
    }

    vector<uint32_t> Font::renderToBitmap(const std::string &str, float&wwidth, float&hheight) {
        uint32_t width = 0;
        uint32_t max_height = 0;
        uint32_t max_descender = 0;
        for (auto wc : str) {
            auto& cchar = getFromCache(wc);
            width += cchar.advance;
            auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        uint32_t height = max_height + max_descender;

        auto bitmap = vector<uint32_t>(width * height, 0);
        int32_t x = 0;
        for (auto wc : str) {
            auto& cchar = getFromCache(wc);
            auto offset = height - cchar.yBearing - max_descender;
            for(int line = 0; line < cchar.height; line++) {
                auto dest = bitmap.data() + (x + cchar.xBearing) + ((line+offset) * width);
                auto src = cchar.bitmap->data() + (line * cchar.width);
                for (int col = 0; col < cchar.width; col++) {
                    auto value = src[col];
                    if (value != 0) { dest[col] = value; }
                }
            }
            x += cchar.advance;
        }
        wwidth = static_cast<float>(width);
        hheight = static_cast<float>(height);
        return bitmap;
    }

    shared_ptr<Image> Font::renderToImage(const Device &device, const string &str) {
        float width, height;
        auto bitmap = renderToBitmap(str, width, height);
        //stbi_write_png("glyph.png", width, height, STBI_rgb_alpha, bitmap.data(), width * STBI_rgb_alpha);
        return make_shared<Image>(device,
                                  str,
                                  width,
                                  height,
                                  static_cast<int>(width * height) * STBI_rgb_alpha,
                                  bitmap.data(),
                                  VK_FORMAT_R8G8B8A8_SRGB,
                                  VK_IMAGE_TILING_OPTIMAL,
                                  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER // don't repeat texture
                                  );
    }

#ifdef FT_FREETYPE_H

    FT_Library Font::library{nullptr};
    uint32_t Font::libraryRef{0};

    Font::Font(const string&_name, uint32_t _size):
        Resource{_name}, path{_name}, size{_size} {
        if (library == nullptr) {
            if (FT_Init_FreeType(&library)) {
                die("Could not initialize FreeType library");
            }
        }
        libraryRef += 1;
        FT_Library_SetLcdFilter(library, FT_LCD_FILTER_LIGHT);
        // Load a font face from a file
        if (FT_New_Face(library, path.c_str(), 0, &face)) {
            die("Could not load font ", path);
        }
        // Set the character size
        // Use 96 DPI for the 2D virtual space
        FT_Set_Char_Size(face,
                         0, static_cast<FT_F26Dot6>(size)*64,
                         96, 96);
    }

    Font::~Font() {
        FT_Done_Face(face);
        libraryRef -= 1;
        if (libraryRef == 0) {
            FT_Done_FreeType(library);
            library = nullptr;
        }
    }

    /*void __saveBitmapAsPGM(const char* filename, uint8_t* bitmap, uint32_t width, uint32_t  rows) {
        auto *file = fopen(filename, "wb");
        if (file) {
            fprintf(file, "P5\n%d %d\n255\n", width, rows);
            fwrite(bitmap, 1, width * rows, file);
            fclose(file);
        } else {
            die("Failed to save PGM file", filename);
        }
    }*/

    void Font::render(CachedCharacter &cachedCharacter, char wchar) {
        // Load and render the glyph for the character
        auto glyph_index = FT_Get_Char_Index(face, wchar);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
            die("Could not render glyph");
        }

        // Get glyph metrics
        auto width = face->glyph->bitmap.width;
        auto height = face->glyph->bitmap.rows;
        cachedCharacter.width = width;
        cachedCharacter.height = height;
        cachedCharacter.advance = static_cast<int32_t>(static_cast<float>(face->glyph->advance.x) / 64.0f);
        cachedCharacter.xBearing = static_cast<int32_t>(face->glyph->bitmap_left);
        cachedCharacter.yBearing = static_cast<int32_t>(face->glyph->bitmap_top);

        // Allocate RGBA bitmap
        cachedCharacter.bitmap = make_unique<vector<uint32_t>>(width * height, 0);

        // Copy temporary bitmap to bitmap vector while converting to RGBA
        auto& srcBitmap = face->glyph->bitmap;
        auto* dstBitmap = cachedCharacter.bitmap->data();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; x++) {
                uint8_t gray = srcBitmap.buffer[y * srcBitmap.pitch + x];
                if (gray != 0) { dstBitmap[y * width + x] = (gray << 24) | (gray << 16) | (gray << 8) | gray; };
            }
        }
        /*string name(1,wchar);
        name = name.append(".pgm");
        __saveBitmapAsPGM(name.c_str(), srcBitmap.buffer, width, height);*/
        //stbi_write_png("glyph.png", width, height, STBI_rgb_alpha, dstBitmap, width * STBI_rgb_alpha);
    }

#endif

}