/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GlTF;

import z0.nodes.Node;

export namespace z0 {

    /*
     * glTF scene loading
     */
    class GlTF {
    public:
        /*
         * Loads a glTF scene
         * @param filepath path of the (binary) glTF file, relative to the application path
         */
        static void load(const shared_ptr<Node>&rootNode, const string& filepath);
    };

}
