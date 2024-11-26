/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.Object;

import z0.Signal;
import z0.Tools;

namespace z0 {

    void Object::connect(const Signal::signal &name, const Signal::Handler& handler) {
        signals[name].connect(handler);
    }

    void Object::connect(const Signal::signal &name, const function<void()>& handler) {
        signals[name].connect([handler](Signal::Parameters*) {
            handler();
        });
    }

    void Object::emit(const Signal::signal &name, Signal::Parameters *params) {
        if (signals.contains(name)) {
            signals[name].emit(params);
        }
    }

}
