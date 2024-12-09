/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Style;

import z0.ui.StyleClassic;

namespace z0 {

    namespace ui {
        Style::Style() {
            font = nullptr;
        }

        shared_ptr<Style> Style::create(const string&NAME) {
            shared_ptr<Style> tmp;
            if (NAME == "vector") {
                tmp = make_shared<StyleClassic>();
            }
            /*else if (NAME == "pixmap") {
                tmp = (GLayout*) new GLayoutPixmap;
            }*/
            if (tmp != nullptr) {
                if (!tmp->init()) {
                    tmp = nullptr;
                }
            }
            return tmp;
        }

        void Style::setOption(const string&NAME, const string&VAL) {
            shared_ptr<GLayoutOption> option;
            for(const auto& opt: options) {
                if (opt->name == NAME) {
                    option = opt;
                    break;
                }
            }
            if (option == nullptr) {
                option = make_shared<GLayoutOption>(NAME);
                options.push_back(option);
            }
            option->value = VAL;
            updateOptions();
        }

        string Style::getOption(const string&NAME) const {
            shared_ptr<GLayoutOption> option;
            for(const auto& opt: options) {
                if (opt->name == NAME) {
                    option = opt;
                    break;
                }
            }
            if (option == nullptr) { return ""; }
            return option->value;
        }
    }

}