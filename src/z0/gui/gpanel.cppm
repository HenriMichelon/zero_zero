/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.GPanel;

import z0.GWidget;

export namespace z0 {

    namespace ui {
        /**
         * %A rectangular widget with only a background (no borders)
         */
        class GPanel: public GWidget {
        public:
            GPanel(): GWidget(PANEL) {};

        protected:
            explicit GPanel(const Type T): GWidget(T) {};
        };
    }

}