#include "z0/renderers/font_renderer.h"

#include <cstring>

namespace z0 {

#ifdef _WIN32

    FIXED FontRenderer::fixedFromDouble(double d) {
        long l = (long) (d * 65536L);
        return *(FIXED *)&l;
    }

    FontRenderer::FontRenderer() {
        mat2.eM11 = fixedFromDouble(1.0);
        mat2.eM12 = fixedFromDouble(0.0);
        mat2.eM21 = fixedFromDouble(0.0);
        mat2.eM22 = fixedFromDouble(1.0);
    }

    FontRenderer::~FontRenderer() {
        DeleteObject(font);
    }

    bool FontRenderer::openFont(const string &NAME, const uint32_t SIZE, const bool B, const bool I, const bool U) {
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

    void FontRenderer::render(CachedCharacter &car, wchar_t CHAR) {
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
