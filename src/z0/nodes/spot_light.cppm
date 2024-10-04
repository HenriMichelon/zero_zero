module;
#include "z0/libraries.h"

export module z0:SpotLight;

import :OmniLight;

export namespace z0 {

    /**
     * A spotlight, such as a spotlight or a lantern.
     */
    class SpotLight : public OmniLight {
    public:
        /**
         * Creates a SpotLight with default parameters
         */
        explicit SpotLight(const string &name = "SpotLight");

        /**
        * Create a SpotLight.
        * @param direction the light direction
        * @param cutOffDegrees the inner cutoff angle that specifies the spotlight's radius, in degrees
        * @param outerCutOffDegrees the outer cutoff angle that specifies the spotlight's radius, in degrees. Everything outside this angle is not lit by the spotlight.
        * @param linear the linear term (see https://learnopengl.com/Lighting/Light-casters)
        * @param quadratic the quadratic term (see https://learnopengl.com/Lighting/Light-casters)
        * @param attenuation the attenuation factor
        * @param color the RGB color and intensity
        * @param specular intensity of the specular blob in objects affected by the light.
        * @param nodeName Node name
        */
        explicit SpotLight(vec3          direction,
                           float         cutOffDegrees,
                           float         outerCutOffDegrees,
                           float         linear,
                           float         quadratic,
                           float         attenuation = 1.0f,
                           vec4          color       = {1.0f, 1.0f, 1.0f, 1.0f},
                           float         specular    = 1.0f,
                           const string &nodeName    = "SpotLight");

        ~SpotLight() override = default;

        /** 
         * Returns the direction of the light
        */
        [[nodiscard]] inline vec3 getDirection() const { return direction; }

        /** 
         * Sets the direction of the light
        */
        inline void setDirection(const vec3 lightDirection) { direction = lightDirection; }

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

    private:
        vec3  direction{0.0f, 0.0f, 1.0f};
        float fov{0.0f};
        float cutOff{cos(radians(10.f))};
        float outerCutOff{cos(radians(15.f))};

        shared_ptr<Node> duplicateInstance() override;
    };

}
