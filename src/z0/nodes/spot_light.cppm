/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.SpotLight;

import z0.Constants;

import z0.nodes.OmniLight;

export namespace z0 {

    /**
     * %A spotlight, such as a spotlight or a lantern.
     */
    class SpotLight : public OmniLight {
    public:
        /**
         * Creates a SpotLight with default parameters
         */
        explicit SpotLight(const string &name = TypeNames[SPOT_LIGHT]);

        /**
        * Create a SpotLight.
        * @param cutOffDegrees the inner cutoff angle that specifies the spotlight's radius, in degrees
        * @param outerCutOffDegrees the outer cutoff angle that specifies the spotlight's radius, in degrees. Everything outside this angle is not lit by the spotlight.
        * @param range Radius of the light and shadows
        * @param color the RGB color and intensity
        * @param nodeName Node name
        */
        explicit SpotLight(float         cutOffDegrees,
                           float         outerCutOffDegrees,
                           float         range,
                           vec4          color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           const string &nodeName    = TypeNames[SPOT_LIGHT]);

        ~SpotLight() override = default;

        /**
         * Sets the inner cutoff angle that specifies the spotlight's radius, in degrees
         */
        inline void setCutOff(const float cutOffDegrees) { cutOff = cos(radians(cutOffDegrees)); }

        /**
         * Returns the inner cutoff value that specifies the spotlight's radius (not the angle!)
         */
        [[nodiscard]] inline float getCutOff() const { return cutOff; }

        /**
         * Sets the outer cutoff angle that specifies the spotlight's radius. Everything outside this angle is not lit by the spotlight.
         */
        void setOuterCutOff(float outerCutOffDegrees);

        /**
         * Returns the outer cutoff value that specifies the spotlight's radius (not the angle!).
         */
        [[nodiscard]] inline float getOuterCutOff() const { return outerCutOff; }

        /**
         * Returns the field of view of the spotlight, in radians
         */
        [[nodiscard]] inline float getFov() const { return fov; }

        void setProperty(const string &property, const string &value) override;

    protected:
        shared_ptr<Node> duplicateInstance() override;

    private:
        float fov{0.0f};
        float cutOff{cos(radians(10.f))};
        float outerCutOff{cos(radians(15.f))};

    };

}
