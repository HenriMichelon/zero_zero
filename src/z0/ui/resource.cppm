/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Resource;

import z0.Object;

export namespace z0 {

    namespace ui {
        /**
         * Super class for style resources descriptions
         */
        class Resource: public Object {
        public:
            explicit Resource(const string& R): res(std::move(R)) {}
            ~Resource() override = default;

            [[nodiscard]] const string& getResource() const { return res; }

        private:
            string res{};
        };
    }

}