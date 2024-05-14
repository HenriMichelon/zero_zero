#include "z0/resources/font.h"

#include <locale>
#include <codecvt>

namespace z0 {

    shared_ptr<Font> Font::create(const string& NAME, const string& ENGINE,
                         uint32_t SIZE, bool B, bool I, bool U) {
        auto font = make_shared<Font>(NAME);
        if (font->fontRenderer.openFont(NAME, SIZE, B, I, U)) {
            return font;
        }
        else {
            die("Font::Create: Cannot open font ", NAME);
            return nullptr;
        }
    }

    uint32_t Font::getWidth(wchar_t CHAR) {
        return getFromCache(CHAR).xAdvance;
    }

    uint32_t Font::getHeight(wchar_t CHAR) {
        auto& rchar = getFromCache(CHAR);
        return rchar.height + rchar.descent;

    }

    uint32_t Font::getWidth(const string &STR) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        auto wideString  = converter.from_bytes(STR);
        uint32_t width = 0;
        for (wchar_t wc : wideString) {
            width += getWidth(wc);
        }
        return width;
    }

    uint32_t Font::getHeight(const string &STR) {
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        auto wideString  = converter.from_bytes(STR);
        uint32_t ymax = 0;
        for (wchar_t wc : wideString) {
            ymax += std::max(ymax, getHeight(wc));
        }
        return ymax;
    }

    void* Font::render(wchar_t CHAR, int32_t &XOFF, int32_t &YOFF)
    {
        CachedCharacter &font = getFromCache(CHAR);
        XOFF = font.leftbearing;
        YOFF = font.descent;
        return font.bitmap;
    }

    CachedCharacter& Font::getFromCache(wchar_t CHAR) {
        if (cache.contains(CHAR)) {
            return cache[CHAR];
        } else {
            auto& cchar = cache[CHAR];
            fontRenderer.render(cchar, CHAR);
            return cchar;
        }
    }


}