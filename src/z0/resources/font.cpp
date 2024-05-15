#include "z0/resources/font.h"

#include <locale>
#include <codecvt>
#include <cstring>

namespace z0 {

    shared_ptr<Font> Font::create(const string& NAME, uint32_t SIZE) {
        auto font = make_shared<Font>(NAME);
        if (font->openFont(NAME, SIZE)) {
            return font;
        }
        die("Font::Create: Cannot open font ", NAME);
        return nullptr;
    }

    void Font::getSize(const std::string &str, uint32_t&width, uint32_t&height) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        auto wideString= converter.from_bytes(str);
        width = 0;
        uint32_t max_height = 0;
        uint32_t max_descender = 0;
        for (wchar_t wc : wideString) {
            auto& cchar = getFromCache(wc);
            width += cchar.advance;
            auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        height = max_height + max_descender;

    }

    Font::CachedCharacter& Font::getFromCache(wchar_t CHAR) {
        if (cache.contains(CHAR)) {
            return cache[CHAR];
        } else {
            auto& cchar = cache[CHAR];
            render(cchar, CHAR);
            return cchar;
        }
    }

    vector<uint8_t> Font::render(const std::string &str, uint32_t&width, uint32_t&height) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        auto wideString= converter.from_bytes(str);
        width = 0;
        uint32_t max_height = 0;
        uint32_t max_descender = 0;
        for (wchar_t wc : wideString) {
            auto& cchar = getFromCache(wc);
            width += cchar.advance;
            auto descender = cchar.height - cchar.yBearing;
            max_height = std::max(max_height, cchar.height);
            max_descender = std::max(max_descender, descender);
        }
        height = max_height + max_descender;

        auto bitmap = vector<uint8_t>(width * height);
        int32_t x = 0;
        for (wchar_t wc : wideString) {
            auto& cchar = getFromCache(wc);
            auto offset = height - cchar.yBearing - max_descender;
            for(int line = 0; line < cchar.height; line++) {
                auto dest = bitmap.data() + (x + cchar.xBearing) + ((line+offset) * width);
                auto src = cchar.bitmap->data() + (line * cchar.width);
                std::memcpy(dest, src, cchar.width);
            }
            x += cchar.advance;
        }
        return bitmap;
    }

#ifdef FT_FREETYPE_H

    FT_Library Font::library{nullptr};
    uint32_t Font::libraryRef{0};

    Font::Font(const std::string &name): Resource{name} {
        if (library == nullptr) {
            if (FT_Init_FreeType(&library)) {
                die("Could not initialize FreeType library");
            }
        }
        libraryRef += 1;
    }

    Font::~Font() {
        FT_Done_Face(face);
        libraryRef -= 1;
        if (libraryRef == 0) { FT_Done_FreeType(library); }
    }

    bool Font::openFont(const std::string &FONTNAME, uint32_t SIZE) {
        // Load a font face from a file
        if (FT_New_Face(library, FONTNAME.c_str(), 0, &face)) {
            die("Could not load font face", FONTNAME);
            return false;
        }
        // Set the character size (width and height in 1/64th points)
        FT_Set_Char_Size(face, 0, static_cast<FT_F26Dot6>(SIZE)*64, 300, 300);
        return true;
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

    void Font::render(z0::Font::CachedCharacter &cachedCharacter, wchar_t wchar) {
        // Load and render the glyph for the character
        auto glyph_index = FT_Get_Char_Index(face, wchar);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER)) {
            die("Could not render glyph");
        }
        auto width = face->glyph->bitmap.width;
        auto height = face->glyph->bitmap.rows;
        cachedCharacter.width = width;
        cachedCharacter.height = height;
        cachedCharacter.advance = static_cast<int32_t>(static_cast<float>(face->glyph->advance.x) / 64.0f);
        cachedCharacter.xBearing = static_cast<int32_t>(face->glyph->bitmap_left);
        cachedCharacter.yBearing = static_cast<int32_t>(face->glyph->bitmap_top);

        cachedCharacter.bitmap = make_unique<vector<uint8_t>>(width * height);
        FT_Bitmap& srcBitmap = face->glyph->bitmap;
        uint8_t* dstBitmap = cachedCharacter.bitmap->data();
        for (int y = 0; y < height; ++y) {
            std::memcpy(dstBitmap + (y * width), srcBitmap.buffer + (y * srcBitmap.pitch), width);
        }
        //__saveBitmapAsPGM("output.pgm", dstBitmap, width, height);
    }

#endif

}