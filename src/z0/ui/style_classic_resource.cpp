/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.StyleClassicResource;

namespace z0::ui {

    StyleClassicResource::StyleClassicResource(const string &RES) : Resource(RES) { splitResString(RES); }

    void StyleClassicResource::splitResString(const string &resourceString) {
        const auto res = split(resourceString, ',');
        if ((!res.empty()) && (!res[0].empty())) {
            width = stof(string{res[0]});
        }
        if ((res.size() > 1) && (!res[1].empty())) {
            height = stof(string{res[1]});
        }
        if (res.size() > 2) {
            if (res[2] == "RAISED") {
                style = RAISED;
            } else if (res[2] == "LOWERED") {
                style = LOWERED;
            } else if (res[2] == "FLAT") {
                style = FLAT;
            }
        }
        if (res.size() > 3) {
            color       = vec4{stof(string{res[3]}), stof(string{res[4]}), stof(string{res[5]}), stof(string{res[6]})};
            customColor = true;
        }
    }

} // namespace z0::ui
