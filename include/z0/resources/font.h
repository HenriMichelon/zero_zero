#pragma once

#include "z0/color.h"
#include "z0/resources/resource.h"

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

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
        static shared_ptr<Font> create(const string& = "", uint32_t = 0);

        /*! Return the width (in pixels) for a character */
        uint32_t getWidth(wchar_t);

        /*! Return the maximum height (in pixels) for a character */
        uint32_t getHeight(wchar_t);

        /*! Return the width (in pixels) for a string */
        uint32_t getWidth(const string&);

        /*! Return the maximum height (in pixels) for a string */
        uint32_t getHeight(const string&);

        /*! Render a string into 8 bpp bitmap.
            Offsets are incremented
            \param string	: string to render
            \param uint32_t	: width
            \param uint32_t	: height
            \return NULL if error or a managed bitmap address
        */
        vector<uint8_t> render(const string&, uint32_t&, uint32_t&);

    private:
        // Already rendered characters
        struct CachedCharacter {
            /*! X Advance in pixels (space to the next character) */
            int32_t	        xAdvance;
            /*! Ascent in pixel (number of pixel above the base line) */
            int32_t	        ascent;
            /*! Descent in pixel (number of pixel below the base line) */
            int32_t	        descent;
            /*! Height in pixels (generaly ascent+descent, but only for horizontal writting) */
            uint32_t	    height;
            /*! Width in pixels*/
            uint32_t	    width;
            /*! Rendered pixmap */
            unique_ptr<vector<uint8_t>> bitmap;
        };
        map<wchar_t, CachedCharacter>   cache;

        CachedCharacter &getFromCache(wchar_t);
        void render(CachedCharacter&, wchar_t);
        bool openFont(const string&, uint32_t);

#ifdef FT_FREETYPE_H
        FT_Face				face;
        static FT_Library 	library;
        static uint32_t     libraryRef;
#endif
    };

}