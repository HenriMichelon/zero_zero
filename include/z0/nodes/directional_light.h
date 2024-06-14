#pragma once

namespace z0 {

   class DirectionalLight: public Light {
    public:
        explicit DirectionalLight(const string name = "DirectionalLight");
        explicit DirectionalLight(vec3 lightDirection,
                                  vec4 color = {1.0f, 1.0f, 1.0f, 1.0f},
                                  float specular = 1.0f,
                                  const string nodeName = "DirectionalLight");
        virtual ~DirectionalLight() {};

        const vec3& getDirection() const { return direction; }
        void setDirection(vec3 lightDirection) { direction = lightDirection; }
        void setPosition(vec3 position) override;

    private:
        vec3 direction{0.0f, .5f, 1.0f};
    };

}