/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.ui.Button;

import z0.Constants;

import z0.ui.Box;

export namespace z0 {
    namespace ui {

        /**
         * %A clickable Box
         */
        class Button : public Box {
        public:
            Button();

        protected:
            bool eventMouseUp(MouseButton B, float X, float Y) override;
        };

    }
}
