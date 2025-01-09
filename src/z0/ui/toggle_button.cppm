/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.ToggleButton;

import z0.Constants;

import z0.ui.CheckWidget;

export namespace z0 {

    namespace ui {
        /**
         * Two states clickable button
         */
        class ToggleButton : public CheckWidget {
        public:
            ToggleButton();

            ~ToggleButton() override = default;

        protected:
            bool eventMouseDown(MouseButton B, float X, float Y) override;
        };
    }
}
