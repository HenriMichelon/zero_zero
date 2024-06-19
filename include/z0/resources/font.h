#pragma once

namespace z0 {

    /**
     * A font resource to render text in bitmaps.
     * A font is a combination of a font file name and a size.
     * Supports true type font files (cf https://github.com/nothings/stb/blob/master/stb_truetype.h)
     * The font size is automatically scaled based on the resolution, from a base resolution of 640x480 
     * (14 is 14 pixels height in this resolution)
     */
    class Font: public Resource {
    public:
        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         * @param size : size in pixels on a base resolution of 640x480
         */
        explicit Font(const string&path, uint32_t size);
        ~Font();

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const string&text, float&width, float&height);

        /**
         *  Renders a string into a RGBA bitmap (stored in CPU memory).
         *  Glyphs are white with alpha channel mapped to the glyphs geometry
         *   @param text : text to render
         *   @param width : width of the resulting bitmap
         *   @param height : height of the resulting bitmap
         *   @return a 32 bits RGBA bitmap stored in CPU memory
        */
        vector<uint32_t> renderToBitmap(const string&text, float& width, float& height);

        /**
         * Renders a string into a RGBA Image resource (stored in GPU memory).
         * Glyphs are white with alpha channel mapped to the glyphs geometry
         * @param device : GPU where the Image will be stored
         * @param text : text to render
         * @result a 32 bits RGBA bitmap stored in GPU memory with a VK_FORMAT_R8G8B8A8_SRGB format, clamped to border and VK_IMAGE_TILING_OPTIMAL tiling
         */
        shared_ptr<Image> renderToImage(const Device&device, const string&text);

        /**
         * Returns the font path. Useful to create another Font resource with a different size
         */
        const string& getFontName() const { return path; }

        /**
         * Returns the font height in pixels (NOT scaled size, but the size given to the Font constructor)
         */
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
        double scaleFontSize(uint32_t baseFontSize);

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