/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.ui.Box;

import z0.ui.Panel;

export namespace z0 {

    namespace ui {
        /**
         * %A rectangular box
         */
        class Box: public Panel {
        public:
            Box(): Panel {BOX} {}

        protected:
            explicit Box(const Type T): Panel{T} {}
        };
    }


}