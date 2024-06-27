#pragma once

namespace z0 {

    /**
     * Directional light from a distance, as from the Sun.
     */
   class DirectionalLight: public Light {
    public:
        /**
         * Creates a DirectionalLight with defaults parameters
         */
        explicit DirectionalLight(const string name = "DirectionalLight");

        /**
         * Create a DirectionalLight
         * @param direction direction of the light rays
         * @param color RGB color and intensity of the light
         * @param specular intensity of the specular blob in objects affected by the light.
         * @param name Node name
         */
        explicit DirectionalLight(vec3 direction,
                                  vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f },
                                  float specular = 1.0f,
                                  const string name = "DirectionalLight");
        virtual ~DirectionalLight() {};

        /** 
         * Returns the direction of the light
        */
        [[nodiscard]] const vec3& getDirection() const { return direction; }

        /** 
         * Sets the direction of the light
        */
        void setDirection(vec3 lightDirection) { direction = lightDirection; }

        void setProperty(const string&property, const string& value) override;

    private:
        vec3 direction{0.0f, .5f, 1.0f};
    };

}