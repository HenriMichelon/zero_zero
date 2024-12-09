/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Frame;

namespace z0 {

    namespace ui {

        void Frame::setText(const string& T) {
            text = T;
            resizeChildren();
            refresh();
        }

    }

}
