module;
#include "z0/libraries.h"

export module Z0:Light;

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
        void setColorAndIntensity(const vec4 color) { colorAndIntensity = color; }

        /**
         * Returns the intensity of the specular blob in objects affected by the light.
         */
        [[nodiscard]] float getSpecularIntensity() const { return specularIntensity; }

        /**
         * Sets the intensity of the specular blob in objects affected by the light.
         */
        void setSpecularIntensity(const float specular) { specularIntensity = specular; }

        /**
         * If `true`, the light will cast real-time shadows. 
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        [[nodiscard]] bool getCastShadows() const { return castShadows; }

        /**
         * Sets to `true` to makes the light cast real-time shadow.
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        void setCastShadow(const bool cast) { castShadows = cast; }

    protected:
        explicit Light(const string &nodeName) :
            Node{nodeName} {
        }

        explicit Light(const vec4 color, const float specular, const string nodeName):
            Node{nodeName},
            colorAndIntensity{color},
            specularIntensity{specular} {
        }

    private:
        vec4  colorAndIntensity{1.0f, 1.0f, 1.0f, 1.0f};
        float specularIntensity{1.0f};
        bool  castShadows{false};
    };

}
