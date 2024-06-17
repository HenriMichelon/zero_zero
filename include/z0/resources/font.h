#pragma once

namespace z0 {


    class Font: public Resource {
    public:
        explicit Font(const string&, uint32_t);
        ~Font();

        /*! Return the size (in pixels) for a string */
        void getSize(const string&, float&, float&);

        /*! Render a string into RGBA bitmap,
         *  Glyph is white with alpha channel mapped to the glyphs geometry
            Offsets are incremented
            \param string	: string to render
            \param uint32_t : size in pixels
            \return
        */
        vector<uint32_t> renderToBitmap(const string&, float&, float&);

        /*! Render a string into 8 bpp bitmap.
            Offsets are incremented
            \param string	: string to render
            \return
        */
        shared_ptr<Image> renderToImage(const Device&, const string&);

        const string& getFontName() const { return path; }
        uint32_t getFontSize() const { return size; }

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
        const string path;
        const uint32_t size;

        CachedCharacter &getFromCache(char);
        void render(CachedCharacter&, char);

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
        float scale;
        int ascent;
        int descent;
        int lineGap;
        stbtt_fontinfo font;
        unique_ptr<vector<unsigned char>> fontBuffer;
#endif
#ifdef FT_FREETYPE_H
        FT_Face				face;
        static FT_Library 	library;
        static uint32_t     libraryRef;
#endif
    };

}