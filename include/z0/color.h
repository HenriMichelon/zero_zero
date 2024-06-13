#pragma once

namespace z0 {

    /**
     * Struct that hold a RGBA color value
     */
    struct Color {
        //! RGBA Color
        vec4 color;

        /**
         * Create a {0.0, 0.0, 0.0, 0.0 } color
         */
        Color() = default;

        /**
         * Create from a RGBA color
         */
        explicit Color(vec4 c) { color = c; }

        /**
         * Create from a RGB color, A=1.0
         */
        explicit Color(vec3 c) { color = vec4{c, 1.0f}; }

        /**
         * Create from a RGBA color
         */
        Color(float r, float g, float b, float a) { color = vec4{r, g, b, a}; }

        /**
         * Create from a RGB color, A=1.0
         */
        Color(float r, float g, float b) { color = vec4{r, g, b, 1.0f}; }

    };

}