#pragma once

#include "z0/color.h"
#include "z0/resources/resource.h"

#include <map>

namespace z0 {


    class Font: public Resource {
    public:
        explicit Font(const string& name);
        ~Font();

        /*! Load a font.
            Default font & size are architecture dependent
            \param String&	: font name, if "" then the platform default font is loaded
            \param uint32_t	: size, if 0 then a default size is selected
            \param bool	: bold
            \param bool	: italic
            \param bool	: underline
	    */
        static shared_ptr<Font> create(const string& = "", uint32_t = 0, bool = false, bool = false, bool = false);

        /*! Return the maximum height (in pixels) for the font */
        uint32_t getHeight() const { return height; };

        /*! Return the maximum descent parameter
            (number of pixels below the baseline) */
        uint32_t getYMin() const { return ymin; }

        /*! Return the width (in pixels) for a character */
        uint32_t getWidth(wchar_t);

        /*! Return the height (in pixels) for a character */
        uint32_t getHeight(wchar_t);

        /*! Return the width (in pixels) for a string */
        uint32_t getWidth(const string&);

        /*! Return the height (in pixels) for a string */
        uint32_t getHeight(const string&);

        /*! Render a character into 8 bpp bitmap.
            Offsets are incremented
            \param char	: char to render
            \param int32_t	: xoffset
            \param int32_t	: yoffset
            \return NULL if error or a managed bitmap address
	     */
        uint8_t* render(wchar_t, int32_t&, int32_t&);

    private:
        struct CachedCharacter {
            /*! X Advance in pixels (space to the next character) */
            int32_t	    xAdvance;
            /*! left side bearing (space between left border of the char & the first char pixel) */
            int32_t	    leftbearing;
            /*! Ascent in pixel (number of pixel above the base line) */
            int32_t	    ascent;
            /*! Descent in pixel (number of pixel below the base line) */
            int32_t	    descent;
            /*! Height in pixels (generaly ascent+descent, but only for horizontal writting) */
            uint32_t	height;
            /*! Width in pixels*/
            uint32_t	width;
            /*! Rendered pixmap */
            uint8_t*    bitmap;
        };

        // Already rendered characters
        map<wchar_t, CachedCharacter>   cache;
        // Maximum height of chars
        uint32_t                        height;
        // font descent (number of lines under the font's baseline)
        uint32_t                        ymin;

        CachedCharacter &getFromCache(wchar_t);
        void render(CachedCharacter&, wchar_t);
        bool openFont(const string&, uint32_t, bool, bool, bool);

#ifdef _WIN32
        MAT2	mat2;
        HFONT	font{nullptr};
        static FIXED   fixedFromDouble(double d);
#endif
    };

}