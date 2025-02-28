/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.ui.Style;

import z0.Tools;

import z0.ui.StyleClassic;

namespace z0::ui {
    Style::Style() { font = nullptr; }

    shared_ptr<Style> Style::create(const string &name) {
        shared_ptr<Style> style;
        if (name == "vector") {
            style = make_shared<StyleClassic>();
        }
        /*else if (NAME == "pixmap") {
            tmp = (GLayout*) new GLayoutPixmap;
        }*/
        if (!style) { die("No style named ", name); }
        style->init();
        style->updateOptions();
        return style;
    }

    void Style::setOption(const string &NAME, const string &VAL) {
        shared_ptr<StyleOption> option;
        for (const auto &opt : options) {
            if (opt->name == NAME) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) {
            option = make_shared<StyleOption>(NAME);
            options.push_back(option);
        }
        option->value = VAL;
        updateOptions();
    }

    string Style::getOption(const string &NAME) const {
        shared_ptr<StyleOption> option;
        for (const auto &opt : options) {
            if (opt->name == NAME) {
                option = opt;
                break;
            }
        }
        if (option == nullptr) {
            return "";
        }
        return option->value;
    }
} // namespace z0::ui
