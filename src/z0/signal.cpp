/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

module z0;

namespace z0 {

    void Signal::emit(Parameters *params) const {
        for (const auto &callable : handlers) {
            (callable.obj->*callable.func)(params);
        }
    }

}
