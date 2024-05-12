#pragma once

#include <utility>

#include "z0/resources/font.h"
#include "z0/renderers/vector_renderer.h"
#include "z0/rect.h"
#include "z0/gui/gresource.h"


namespace z0 {

    class GWidget;

    class GStyle: public Object
    {
    public:
        GStyle();
        virtual ~GStyle() = default;

        /*! Create a new layout.
          \param	string	: layout name
          \return 	nullptr on error (unknown layout)
        */
        static shared_ptr<GStyle> create(const string& = "vector");

        /*! Create a resource from a resource description string.
          \param	string : string that describe the resources of a widget
          \return 	NEVER return nullptr
        */
        virtual void addResource(GWidget&, const string&) = 0;

        /* Set a layout specific option
            \param	string	: option name
            \param	string	: value
        */
        virtual void setOption(const string&, const string&);

        /* Read a layout specific option
            \param	string	: option name
            \return option value
        */
        string getOption(const string&);

        /*! Draw a widget.
          \param	GWidget	: widget to draw
          \param	GResource : resources used for drawing this widget
          \param	bool : TRUE = before drawing children, FALSE = after
        */
        virtual void draw(const GWidget&, GResource&, VectorRenderer&, bool) const = 0;


        /*! Resize a widget.
          \param	GWidget	: widget to draw
          \param	GResource : resources used for resizing this widget
        */
        virtual void resize(GWidget&, Rect&, GResource&) = 0;


        /*! Return the default font for the layout.
          \param	Font	: font to use for the layout
         */
        shared_ptr<Font> getFont() const { return font; };

    protected:
        shared_ptr<Font>	font;
        virtual bool init() = 0;
        virtual void updateOptions() = 0;

    private:
        class GLayoutOption
        {
        public:
            string name;
            string value;
            GLayoutOption(string N): name(std::move(N)) {};
        };

        list<shared_ptr<GLayoutOption>> options;
    };

}