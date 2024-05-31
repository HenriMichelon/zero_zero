#pragma once

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

        /*! Render a string into RGBA bitmap,
         *  Glyph is white with alpha channel mapped to the glyphs geometry
            Offsets are incremented
            \param string	: string to render
            \param uint32_t : size in pixels
            \return
        */
        vector<uint32_t> renderToBitmap(const string&, uint32_t&, uint32_t&);

        /*! Render a string into 8 bpp bitmap.
            Offsets are incremented
            \param string	: string to render
            \return
        */
        shared_ptr<Image> renderToImage(const Device&, const string&);

    private:
        // Already rendered characters
        struct CachedCharacter {
            int32_t	                     advance;
            int32_t	                     xBearing;
            int32_t	                     yBearing;
            uint32_t	                 width;
            uint32_t	                 height;
            unique_ptr<vector<uint32_t>> bitmap;
        };
        map<char, CachedCharacter> cache;

        CachedCharacter &getFromCache(char);
        void render(CachedCharacter&, char);

#ifdef FT_FREETYPE_H
        FT_Face				face;
        static FT_Library 	library;
        static uint32_t     libraryRef;
#endif
    };

}