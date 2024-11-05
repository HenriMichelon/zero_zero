/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

module z0;

import :Signal;
import :Object;

namespace z0 {

    void Object::connect(const Signal::signal &name, Object *object, Signal::Handler handler) {
        signals[name].connect(object, handler);
    }

    void Object::disconnect(const Signal::signal &name, Object *object, Signal::Handler handler) {
        if (signals.contains(name)) {
            signals[name].disconnect(object, handler);
        }
    }

    void Object::emit(const Signal::signal &name, Signal::Parameters *params) {
        if (signals.contains(name)) {
            signals[name].emit(params);
        }
    }

}
