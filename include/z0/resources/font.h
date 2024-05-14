#pragma once

#include "z0/resources/resource.h"
#include "z0/renderers/font_renderer.h"

#include <map>

namespace z0 {


    class Font: public Resource {
    public:
        explicit Font(const string& name): Resource{name} {};

        /*! Load a font.
		Default font & size are architecture dependent
		\param String&	: font name, if "" then the engine default font is loaded
		\param String&	: engine name, if "" then the os/wm default engine is used
		\param uint32_t	: size, if 0 then a default size is selected
		\param bool	: bold
		\param bool	: italic
		\param bool	: underline
	*/
        static shared_ptr<Font> create(const string& = "", const string& = "",
                                uint32_t = 0, bool = false, bool = false, bool = false);

        /*! Select the text (pen) color
            default color is (0, 0, 0)	
        */
        void setTextColor(const Color&);

        /*! Return the maximum height (in pixels) for the font */
        uint32_t getHeight() const { return fontRenderer.getHeight(); };

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
            \return NULL if error
	     */
        void* render(wchar_t, int32_t&, int32_t&);

    private:
        Color	                        textColor;
        FontRenderer                    fontRenderer;
        map<wchar_t, CachedCharacter>   cache;

        CachedCharacter &getFromCache(wchar_t);


    };

}