#include "z0/resources/font.h"

#include <locale>
#include <codecvt>
#include <cstring>

namespace z0 {

    shared_ptr<Font> Font::create(const string& NAME, const string& ENGINE,
                         uint32_t SIZE, bool B, bool I, bool U) {
        auto font = make_shared<Font>(NAME);
        if (font->openFont(NAME, SIZE, B, I, U)) {
            return font;
        }
        die("Font::Create: Cannot open font ", NAME);
        return nullptr;
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

    Font::CachedCharacter& Font::getFromCache(wchar_t CHAR) {
        if (cache.contains(CHAR)) {
            return cache[CHAR];
        } else {
            auto& cchar = cache[CHAR];
            render(cchar, CHAR);
            return cchar;
        }
    }

#ifdef _WIN32

    FIXED Font::fixedFromDouble(double d) {
        long l = (long) (d * 65536L);
        return *(FIXED *)&l;
    }

    Font::Font(const string& name): Resource{name} {
        mat2.eM11 = fixedFromDouble(1.0);
        mat2.eM12 = fixedFromDouble(0.0);
        mat2.eM21 = fixedFromDouble(0.0);
        mat2.eM22 = fixedFromDouble(1.0);
    }

Font::~Font() {
        DeleteObject(font);
    }

    bool Font::openFont(const string &NAME, const uint32_t SIZE, const bool B, const bool I, const bool U) {
        if (font != nullptr) { DeleteObject(font); }

        string name;
        if (NAME.empty()) {
            name = "Verdana";
        }
        else {
            name = NAME;
        }

        uint32_t size = SIZE;
        if (!size) { size = 11; }

        int weight = FW_NORMAL;
        if (B) { weight = FW_BOLD; }

        HDC dc = GetDC(nullptr);
        if (!dc) return false;
        font = CreateFont(-size,
                          0,
                          0,
                          0,
                          weight,
                          I,
                          U,
                          FALSE,
                          DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS,
                          CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE,
                          name.c_str());
        if (SelectObject(dc, font) == nullptr) {
            die("FontRenderer::CreateFont error");
            ReleaseDC(nullptr, dc);
            return false;
        }
        TEXTMETRIC textmetric;
        GetTextMetrics(dc, &textmetric);
        height = textmetric.tmAscent + textmetric.tmDescent;
        ymin = textmetric.tmDescent;
        ReleaseDC(nullptr, dc);
        return font != nullptr;
    }

    void Font::render(CachedCharacter &car, wchar_t CHAR) {
        GLYPHMETRICS gm;

        HDC dc = GetDC(nullptr);
        if (!dc) return;
        if (SelectObject(dc, font) == nullptr)	{
            die("FontRenderer::render(): SelectObject() error\n");
            ReleaseDC(nullptr, dc);
            return;
        }

        UINT dchar = CHAR;
        uint32_t size = GetGlyphOutlineA(dc,
                                         dchar,
                                         GGO_BITMAP, // + GGO_GLYPH_INDEX,
                                         &gm,
                                         0,
                                         nullptr,
                                         &mat2);
        if (size == GDI_ERROR)	{
            size = GetGlyphOutline(dc,
                                   20,
                                   GGO_BITMAP,
                                   &gm,
                                   0,
                                   nullptr,
                                   &mat2);
            if (size == GDI_ERROR)	{
                ReleaseDC(nullptr, dc);
                return;
            }
        }

        car.height = gm.gmBlackBoxY;
        if (size)	{
            if ((size*4) > (gm.gmBlackBoxX*gm.gmBlackBoxY))
                car.width = gm.gmBlackBoxX+(size*4 - gm.gmBlackBoxX*gm.gmBlackBoxY)/gm.gmBlackBoxY;
            else
                car.width = gm.gmBlackBoxX-(gm.gmBlackBoxX*gm.gmBlackBoxY - size*4)/gm.gmBlackBoxY;

            car.bitmap = new uint8_t[car.width*car.height];
            auto *bitmap = new uint8_t[size];
            GetGlyphOutline(dc, dchar, GGO_BITMAP, &gm, size, bitmap, &mat2);
            std::memset(car.bitmap, 0, car.width*car.height);

            uint32_t idx = 0;
            for (uint32_t y=0; y<car.height; y++) {
                for (uint32_t x=0; x<(car.width/8); x++) {
                    for (uint8_t i=0; i<8; i++) {
                        car.bitmap[y*car.width + x*8 + (7-i)] = (bitmap[idx] >> i) & 1;
                    }
                    idx++;
                }
                idx += (car.width/4) - (car.width/8);
            }
            delete []bitmap;
        }
        else {
            car.width = gm.gmBlackBoxX;
            car.bitmap = new uint8_t[car.width*car.height];
            std::memset(car.bitmap, 0, car.width*car.height);
        }
        car.leftbearing =  gm.gmptGlyphOrigin.x;
        car.xAdvance = gm.gmCellIncX; // + car.leftbearing;
        car.ascent = gm.gmptGlyphOrigin.y;
        car.descent = gm.gmBlackBoxY-gm.gmptGlyphOrigin.y;
        ReleaseDC(nullptr, dc);
    }

#endif



}