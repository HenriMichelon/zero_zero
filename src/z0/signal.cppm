/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Signal;

namespace z0 {

    /**
     * %A signal of an Object
     */
    export class Signal {
    public:
        using signal = string;

        /**
         * Lambda expression who answer to emitted signals
         */
        using Handler = function<void(void*)>;

        /**
         * Connects a member function to the signal
        */
        inline void connect(const Handler &handler) {
            handlers.push_back(handler);
        }

        /**
         * Emits the signal by calling all the connected functions in the connect order
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(void* params) const;

    private:
        list<Handler> handlers;
    };

}