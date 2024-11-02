/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0:OmniLight;

import :Light;

export namespace z0 {

    /**
     * Omnidirectional light, such as a light bulb or a candle
     */
    class OmniLight : public Light {
    public:
        /**
         * Creates an OmniLight with default parameters
         */
        explicit OmniLight(const string &name = TypeNames[OMNI_LIGHT], Type type = OMNI_LIGHT);

        /**
         * Create an OmniLight.
         * @param range The light's radius
         * @param color the RGB color and intensity
         * @param nodeName Node name
         */
        explicit OmniLight(float         range,
                           vec4          color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           const string &nodeName    =TypeNames[OMNI_LIGHT],
                           Type          type        = OMNI_LIGHT);

        ~OmniLight() override = default;

        /**
         * Returns the light range (default 10m)
         */
        [[nodiscard]] inline float getRange() const { return range; }

        /**
         * Sets the light range
         */
        inline void setRange(const float range) { this->range = range; }

        /**
         * Returns the light near clipping distance for the shadows (default 0.1m)
         */
        [[nodiscard]] inline float getNearClipDistance() const { return near; }

        void setProperty(const string &property, const string &value) override;

    protected:
        // Maximum distance of lighting
        float range{10.0f};
        // clipping distance for shadows
        float near{0.1f};
    };

}
