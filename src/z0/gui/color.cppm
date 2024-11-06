/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Color;

export namespace z0 {

    /**
     * Struct that hold a RGBA color value
     */
    struct Color {
        //! RGBA Color
        vec4 color{0.0f, 0.0f, 0.0f, 1.0f};

        /**
         * Create a {0.0, 0.0, 0.0, 0.0 } color
         */
        Color() = default;

        /**
         * Create from an RGBA color
         */
        explicit Color(const vec4 c) { color = c; }

        /**
         * Create from an RGB color, A=1.0
         */
        explicit Color(const vec3 c) { color = vec4{c, 1.0f}; }

        /**
         * Create from an RGBA color
         */
        Color(const float r, const float g, const float b, const float a) { color = vec4{r, g, b, a}; }

        /**
         * Create from an RGB color, A=1.0
         */
        Color(const float r, const float g, const float b) { color = vec4{r, g, b, 1.0f}; }

        inline bool operator == (const Color&other) const {
            return color == other.color;
        }

    };

}
