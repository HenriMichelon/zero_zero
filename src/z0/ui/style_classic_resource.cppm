/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.StyleClassicResource;

import z0.Tools;

import z0.ui.Resource;

export namespace z0 {

    namespace ui {

        class StyleClassicResource : public Resource {
        public:
            enum Style {
                FLAT,
                RAISED,
                LOWERED
            };

            Style style{FLAT};
            float width{0};
            float height{0};
            bool flat{false};

            explicit StyleClassicResource(const string& RES);

        private:
            void splitResString(const string& RES);
        };
    }
}
