/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.OmniLight;

import z0.nodes.Light;

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
         * @param type Omni or Spot light
         */
        explicit OmniLight(float         range,
                           vec4          color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           const string &nodeName    =TypeNames[OMNI_LIGHT],
                           Type          type        = OMNI_LIGHT);

        ~OmniLight() override = default;

        /**
         * Returns the light range (default 10m)
         */
        [[nodiscard]] inline auto getRange() const { return range; }

        /**
         * Sets the light range
         */
        void setRange(float range);

        /**
         * Returns the light near clipping distance for the shadows (default 0.1m)
         */
        [[nodiscard]] inline auto getNearClipDistance() const { return near; }

        void setProperty(const string &property, const string &value) override;

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    private:
        // Maximum distance of lighting
        float range{10.0f};
        // clipping distance for shadows
        float near{0.1f};
    };

}
