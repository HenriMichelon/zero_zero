/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Style;

import z0.Object;

import z0.resources.Font;

import z0.ui.Rect;
import z0.ui.Resource;
import z0.ui.Widget;

import z0.vulkan.VectorRenderer;

namespace z0 {

    namespace ui {
        /**
         * UI Widget drawing style base class
         */
        export class Style : public Object {
        public:

            Style();

            ~Style() override = default;

            /**
             * Creates a new UI drawing style renderer.
             *     @param name	: style name
            */
            [[nodiscard]] static shared_ptr<Style> create(const string & name= "vector");

            /**
             * Create a resource from a resources description string.
             * @param widget : widget to ass resources string to
             * @param resources : string that describe the resources of a widget
            */
            virtual void addResource(Widget &widget, const string &resources) = 0;

            /**
             * Sets a style-specific option
            */
            virtual void setOption(const string &name, const string &value);

            /**
             * Returns a style-specific option value
            */
            [[nodiscard]] string getOption(const string &name) const;

            /* Draws a widget.
             * @param widget : widget to draw
             * @param resources : resources used for drawing this widget
             * @param when : `true` = before drawing children, `false` = after
            */
            virtual void draw(const Widget &widget, Resource &resources, VectorRenderer &rendere, bool when) const = 0;

            /**
             * Adjusts a widget size to style specific constraints
            */
            virtual void resize(Widget &widget, Rect &rect, Resource &resources) = 0;

            /**
             * Returns the default font for the style.
             */
            [[nodiscard]] shared_ptr<Font> getFont() const { return font; }

        protected:
            shared_ptr<Font> font;

            virtual void init() {}

            virtual void updateOptions() = 0;

        private:
            class StyleOption {
            public:
                string name;
                string value;
                explicit StyleOption(const string &N):
                    name(std::move(N)) {
                }
            };

            list<shared_ptr<StyleOption>> options;
        };
    }
}
