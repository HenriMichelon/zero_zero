/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.ui.Panel;

import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        /**
         * %A rectangular widget with only a background (no borders)
         */
        class Panel: public Widget {
        public:
            Panel(): Widget(PANEL) {}

        protected:
            explicit Panel(const Type T): Widget(T) {}
        };
    }

}