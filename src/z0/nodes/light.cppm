module;
#include "z0/libraries.h"

export module z0:Light;

import :Node;

export namespace z0 {

    /**
     * Base class for different kinds of light nodes
     */
    class Light : public Node {
    public:
        ~Light() override = default;

        /**
         * Returns the RGB color and the intensity factor
         */
        [[nodiscard]] const vec4 &getColorAndIntensity() const { return colorAndIntensity; }

        /**
         * Sets the RGB color and the intensity factor
         */
        void setColorAndIntensity(const vec4 colorAndIntensity) { this->colorAndIntensity = colorAndIntensity; }

        /**
         * Returns the intensity of the specular blob in objects affected by the light.
         */
        [[nodiscard]] float getSpecularIntensity() const { return specularIntensity; }

        /**
         * Sets the intensity of the specular blob in objects affected by the light.
         */
        void setSpecularIntensity(const float specularIntensity) { this->specularIntensity = specularIntensity; }

        /**
         * If `true`, the light will cast real-time shadows. 
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        [[nodiscard]] bool getCastShadows() const { return castShadows; }

        /**
         * Sets to `true` to makes the light cast real-time shadow.
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        void setCastShadow(const bool castShadows) { this->castShadows = castShadows; }

    protected:
        explicit Light(const string &nodeName, const Type type = LIGHT) :
            Node{nodeName, type} {
        }

        explicit Light(const vec4 color, const float specular, const string& nodeName, const Type type = LIGHT):
            Node{nodeName, type},
            colorAndIntensity{color},
            specularIntensity{specular} {
        }

    private:
        vec4  colorAndIntensity{1.0f, 1.0f, 1.0f, 1.0f};
        float specularIntensity{1.0f};
        bool  castShadows{false};
    };

}
