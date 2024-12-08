/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module z0.Signal;

import z0.Application;

namespace z0 {

    void Signal::emit(Parameters *params) const {
        for (const auto &callable : handlers) {
            callable(params);
        }
    }

    void Signal::_emitDeferred(Parameters* params) const {
        Application::get().callDeferred([this, params] {
            for (const auto &callable : handlers) {
                callable(params);
            }
        });
    }

}
