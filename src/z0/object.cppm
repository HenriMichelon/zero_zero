/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Object;

import z0.Signal;

export namespace z0 {

    /**
     * Base class for anything
     */
    class Object {
    public:
        /**
         * Connects a signal by name to a function
         * @param name signal name.
         * @param handler the member function to call when emit() is called
        */
        void connect(const Signal::signal &name, const Signal::Handler& handler);

        /**
         * Connects a signal by name to a function
         * @param name signal name.
         * @param handler the member function to call when emit() is called
        */
        void connect(const Signal::signal &name, const function<void()>& handler);

        /**
         * Emits a signal by name by calling all the connected function in the connect order
         * @param name signal name
         * @param params parameters to pass to the function connected to the signal
         */
        void emit(const Signal::signal &name, void *params = nullptr);

        /**
         * Converts the objet to a readable text
         */
        [[nodiscard]] virtual string toString() const { return "??"; }

        friend ostream &operator<<(ostream &os, const Object &obj) {
            os << obj.toString();
            return os;
        }

        Object() = default;
        virtual ~Object() = default;

    private:
        map<string, Signal> signals;
    };

}
