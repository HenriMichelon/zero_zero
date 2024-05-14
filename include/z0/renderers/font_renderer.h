#pragma once

#include "z0/color.h"

namespace z0 {

    struct CachedCharacter {
        /*! X Advance in pixels (space to the next character) */
        int32_t	    xAdvance;
        /*! left side bearing (space between left border of the char & the first char pixel) */
        int32_t	    leftbearing;
        /*! Ascent in pixel (number of pixel above the base line) */
        int32_t	    ascent; // bboxtop
        /*! Descent in pixel (number of pixel below the base line) */
        int32_t	    descent; // yMin
        /*! Height in pixels (normaly ascent+descent, but only for horizontal writting) */
        uint32_t	height;
        /*! Width in pixels*/
        uint32_t	width;
        /*! Rendered pixmap */
        uint8_t*    bitmap;
    };

    class FontRenderer {
    public:
        FontRenderer();
        virtual ~FontRenderer();

        /*! Load a font.
            Default font & size are architecture dependent
            \param string&	: font name, if "" then a default font is loaded
            \param uint32_t	: size, if 0 then a default size is selected
            \param bool	: bold
            \param bool	: italic
            \param bool	: underline
        */
        virtual bool openFont(const string&, uint32_t, bool, bool, bool);

        /*! Return the maximum height for the font */
        uint32_t getHeight() const { return height; };

        /*! Return the maximum descent parameter
            (number of pixels below the baseline) */
        uint32_t getYMin() const { return ymin; };

        /*! Render a char. */
        virtual void render(CachedCharacter&, wchar_t);

    protected:
        uint32_t height; // Maximum height of chars
        uint32_t ymin; // font descent (number of lines under the font's baseline)

    private:
#ifdef _WIN32
        MAT2	mat2;
        HFONT	font{nullptr};
        static FIXED   fixedFromDouble(double d);
#endif
    };

}
