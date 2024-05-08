#pragma once

#include "z0/resources/font.h"
#include "z0/gui/grect.h"
#include "z0/gui/gresource.h"

#include <string>
#include <list>
using namespace std;

namespace z0 {

    class GWidget;

    class GLayout
    {
    public:
        GLayout();
        virtual ~GLayout() = default;

        /*! Create a new layout.
          \param	string	: layout name
          \return 	nullptr on error (unknown layout)
        */
        static shared_ptr<GLayout> create(const string& = "vector");

        /*! Create a resource from a resource description string.
          \param	string : string that describe the resources of a widget
          \return 	NEVER return nullptr
        */
        virtual void addResource(GWidget&, const string& = "") = 0;

        /*! Delete a resource created with AddResource() */
        virtual void deleteResource(GResource*) = 0;

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
          \param	GRect : unused
          \param	bool : TRUE = before drawing childs, FALSE = after
        */
        virtual void draw(GWidget&, GResource&, const GRect&, bool) = 0;


        /*! Resize a widget.
          \param	GWidget	: widget to draw
          \param	GResource : resources used for resizing this widget
          \param	uint32_t	: width
          \param	uint32_t	: height
        */
        virtual void resize(GWidget&, GRect&, GResource&) = 0;


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
            GLayoutOption(const string&N): name(N) {};
        };

        list<shared_ptr<GLayoutOption>> options;
    };

}