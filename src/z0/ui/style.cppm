/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Style;

import z0.resources.Font;
import z0.Object;

import z0.VectorRenderer;

import z0.ui.Rect;
import z0.ui.Resource;
import z0.ui.Widget;

namespace z0 {

    namespace ui {
        /**
         * Widget drawing base class
         */
        export class Style : public Object {
        public:

            Style();

            ~Style() override = default;

            /* Create a new layout.
                  string	: layout name
                   nullptr on error (unknown layout)
            */
            [[nodiscard]] static shared_ptr<Style> create(const string & = "vector");

            /* Create a resource from a resource description string.
                  string : string that describe the resources of a widget
                   NEVER return nullptr
            */
            virtual void addResource(Widget &, const string &) = 0;

            /* Set a layout specific option
                    string	: option name
                    string	: value
            */
            virtual void setOption(const string &, const string &);

            /* Read a layout specific option
                    string	: option name
                 option value
            */
            [[nodiscard]] string getOption(const string &);

            /* Draw a widget.
                  Widget	: widget to draw
                  Resource : resources used for drawing this widget
                  bool : TRUE = before drawing children, FALSE = after
            */
            virtual void draw(const Widget &, Resource &, VectorRenderer &, bool) const = 0;

            /* Resize a widget.
                  Widget	: widget to draw
                  Resource : resources used for resizing this widget
            */
            virtual void resize(Widget &, Rect &, Resource &) = 0;

            /* Return the default font for the layout.
                  Font	: font to use for the layout
             */
            [[nodiscard]] shared_ptr<Font> getFont() const { return font; }

        protected:
            shared_ptr<Font> font;

            virtual bool init() = 0;

            virtual void updateOptions() = 0;

        private:
            class GLayoutOption {
            public:
                string name;
                string value;

                explicit GLayoutOption(const string &N):
                    name(std::move(N)) {
                }
            };

            list<shared_ptr<GLayoutOption>> options;
        };
    }
}
