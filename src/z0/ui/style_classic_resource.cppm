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

            explicit StyleClassicResource(const string& RES):
                Resource(RES) {
                splitResString(RES);
            }

        private:
            void splitResString(const string& RES) {
                auto res = split(RES, ',');
                if ((!res.empty()) && (!res[0].empty())) {
                    width = stof(string{res[0]});
                }
                if ((res.size() > 1) && (!res[1].empty())) {
                    height = stof(string{res[1]});
                }
                if ((res.size() > 2) && (!res[2].empty())) {
                    if (res[2] == "RAISED") { style = RAISED; }
                    if (res[2] == "LOWERED") { style = LOWERED; }
                    if (res[2] == "FLAT") { style = FLAT; }
                }
                if ((res.size() > 3) && (!res[3].empty())) {
                    flat = (res[3] == "FLAT");
                }
            }
        };
    }
}
