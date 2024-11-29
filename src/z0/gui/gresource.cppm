/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GResource;

import z0.Object;

export namespace z0 {

    namespace ui {
        /**
         * Super class for style resources descriptions
         */
        class GResource: public Object {
        public:
            explicit GResource(const string& R): res(std::move(R)) {};
            ~GResource() override = default;

            [[nodiscard]] const string& Resource() const { return res; };

        private:
            string res;
        };
    }

}