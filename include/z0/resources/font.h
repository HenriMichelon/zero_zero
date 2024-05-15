#pragma once

#include "z0/color.h"
#include "z0/resources/resource.h"

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace z0 {


    class Font: public Resource {
    public:
        explicit Font(const string&, uint32_t);
        ~Font();

        /*! Load a font.
            Default font & size are architecture dependent
            \param String&	: font name, if "" then the platform default font is loaded
            \param uint32_t	: size, if 0 then a default size is selected
	    */
        static shared_ptr<Font> create(const string& = "", uint32_t = 0);

        /*! Return the size (in pixels) for a string */
        void getSize(const string&, uint32_t&, uint32_t&);

        /*! Render a string into 8 bpp bitmap.
            Offsets are incremented
            \param string	: string to render
            \param uint32_t : size in pixels
            \return NULL if error or a managed bitmap address
        */
        vector<uint8_t> render(const string&, uint32_t&, uint32_t&);

    private:
        // Already rendered characters
        struct CachedCharacter {
            int32_t	        advance;
            int32_t	        xBearing;
            int32_t	        yBearing;
            uint32_t	    width;
            uint32_t	    height;
            unique_ptr<vector<uint8_t>> bitmap;
        };
        map<wchar_t, CachedCharacter>   cache;

        CachedCharacter &getFromCache(wchar_t);
        void render(CachedCharacter&, wchar_t);

#ifdef FT_FREETYPE_H
        FT_Face				face;
        static FT_Library 	library;
        static uint32_t     libraryRef;
#endif
    };

}