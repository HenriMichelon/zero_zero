/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <stb_truetype.h>
#include "z0/libraries.h"
#include "z0/vulkan.h"

export module z0.resources.Font;

import z0.resources.Image;
import z0.resources.Resource;

export namespace z0 {

    /**
     * %A font resource to render text in bitmaps.
     * %A font is a combination of a font file name and a size.
     * Supports true type font files (cf https://github.com/nothings/stb/blob/master/stb_truetype.h).
     * The font size is automatically scaled based on the resolution, from a base resolution of 1920x1080
     * (14 is 14 pixels height in this resolution)
     */
    class Font : public Resource {
    public:
        /**
         * Creates a font resource
         * @param path : font file path, relative to the application working directory
         * @param size : height in pixels on a base resolution of 1920x1080
         */
        explicit Font(const string &path, uint32_t size);

        explicit Font(const Font &font, uint32_t size);

        ~Font() override;

        /**
         * Returns the size (in pixels) for a string.
         */
        void getSize(const string &text, float &width, float &height);

        /**
         *  Renders a string into an RGBA bitmap (stored in CPU memory).
         *  Glyphs are white with alpha channel mapped to the glyphs geometry
         *   @param text : text to render
         *   @param wwidth : width of the resulting bitmap
         *   @param hheight : height of the resulting bitmap
         *   @return 32 bits RGBA bitmap stored in CPU memory
        */
        [[nodiscard]] vector<uint32_t> renderToBitmap(const string &text, float &wwidth, float &hheight);

        [[nodiscard]] shared_ptr<Image> renderToImage(VkCommandPool commandPool, const string &text);

        /**
         * Returns the font path. Useful to create another Font resource with a different size
         */
        [[nodiscard]] inline const string &getFontName() const { return path; }

        /**
         * Returns the font height in pixels (NOT scaled size, but the size given to the Font constructor)
         */
        [[nodiscard]] inline uint32_t getFontSize() const { return size; }

    private:
        // Already rendered characters
        struct CachedCharacter {
            int32_t                      advance;
            int32_t                      xBearing;
            int32_t                      yBearing;
            uint32_t                     width;
            uint32_t                     height;
            unique_ptr<vector<uint32_t>> bitmap;
        };

        map<char, CachedCharacter> cache;
        const string               path;
        const uint32_t             size;

        [[nodiscard]] CachedCharacter &getFromCache(char c);

        void render(CachedCharacter &, char) const;

        [[nodiscard]] uint32_t scaleFontSize(uint32_t baseFontSize) const;

#ifdef __STB_INCLUDE_STB_TRUETYPE_H__
        float                             scale;
        int                               ascent;
        int                               descent;
        int                               lineGap;
        int                               height;
        stbtt_fontinfo                    font;
        unique_ptr<vector<unsigned char>> fontBuffer;
#endif
    };

}
