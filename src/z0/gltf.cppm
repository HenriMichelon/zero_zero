/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GlTF;

import z0.Node;

export namespace z0 {

    /**
     * glTF scene loading
     */
    class GlTF {
    public:
        /**
         * Load a glTF scene
         * @param filepath path of the (binary) glTF file, relative to the application path
         */
        [[nodiscard]] static shared_ptr<Node> load(const string& filepath);
    };

}
