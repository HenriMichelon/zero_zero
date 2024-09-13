module;
#include "z0/libraries.h"

export module Z0:SpotLight;

import :Tools;
import :Node;
import :OmniLight;

export namespace z0 {

    /**
     * A spotlight, such as a spotlight or a lantern.
     */
    class SpotLight: public OmniLight {
    public:
        /**
         * Creates a SpotLight with default parameters
         */
        explicit SpotLight(const string& name = "SpotLight"): OmniLight{name} {};

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
        explicit SpotLight(const vec3 direction,
                           const float cutOffDegrees,
                           const float outerCutOffDegrees,
                           const float linear,
                           const float quadratic,
                           const float attenuation = 1.0f,
                           const vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           const float specular = 1.0f,
                           const string nodeName = "SpotLight"):
            OmniLight{linear, quadratic, attenuation, color, specular, nodeName},
            direction{direction},
            fov{radians(outerCutOffDegrees)},
            cutOff{cos(radians(cutOffDegrees))},
            outerCutOff{cos(fov)}
        { }

        ~SpotLight() override = default;

        /** 
         * Returns the direction of the light
        */
        [[nodiscard]] vec3& getDirection() { return direction; }

        /** 
         * Sets the direction of the light
        */
        void setDirection(const vec3 lightDirection) { direction = lightDirection; }

        /**
         * Sets the inner cutoff angle that specifies the spotlight's radius, in degrees
         */
        void setCutOff(const float cutOffDegrees) {
            cutOff = cos(radians(cutOffDegrees));
        }

        /**
         * Returns the inner cutoff value that specifies the spotlight's radius (not the angle!)
         */
        [[nodiscard]] float getCutOff() const {return cutOff;}

        /**
         * Sets the outer cutoff angle that specifies the spotlight's radius. Everything outside this angle is not lit by the spotlight.
         */
        void setOuterCutOff(const float outerCutOffDegrees) {
            fov = radians(outerCutOffDegrees);
            outerCutOff = cos(fov);
        }

        /**
         * Returns the outer cutoff value that specifies the spotlight's radius (not the angle!).
         */
        [[nodiscard]] float getOuterCutOff() const {return outerCutOff;}

        /**
         * Returns the field of view of the spotlight, in radians
         */
        [[nodiscard]] float getFov() const { return fov; }

    private:
        vec3 direction{0.0f, 0.0f, 1.0f};
        float fov{0.0f};
        float cutOff { cos(radians(10.f)) };
        float outerCutOff { cos(radians(15.f)) };

        shared_ptr<Node> duplicateInstance() override {
          return make_shared<SpotLight>(*this);
        }
    };

}