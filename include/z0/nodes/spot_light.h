#pragma once

namespace z0 {

    /**
     * A spotlight, such as a spotlight or a lantern.
     */
    class SpotLight: public OmniLight {
    public:
        explicit SpotLight(const string name = "SpotLight"): OmniLight{name} {};
        explicit SpotLight(vec3 lightDirection,
                           float cutOffDegrees,
                           float outerCutOffDegrees,
                           float linear,
                           float quadratic,
                           float attenuation = 1.0f,
                           vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                           float specular = 1.0f,
                           const string nodeName = "SpotLight");
        virtual ~SpotLight() = default;

        vec3& getDirection() { return direction; }
        void setDirection(vec3 lightDirection) { direction = lightDirection; }
        void setCutOff(float cutOffDegrees);
        float getCutOff() const {return cutOff;}
        void setOuterCutOff(float outerCutOffDegrees);
        float getOuterCutOff() const {return outerCutOff;}
        float getFov() const { return fov; }

    private:
        vec3 direction{0.0f, 0.0f, 1.0f};
        float fov{0.0f};
        float cutOff { cos(radians(10.f)) };
        float outerCutOff { cos(radians(15.f)) };
        shared_ptr<Node> duplicateInstance() override;
    };

}