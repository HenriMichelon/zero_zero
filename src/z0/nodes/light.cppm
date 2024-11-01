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

        // For the fragment shader
        enum LightType {
            LIGHT_UNKNOWN     = -1,
            LIGHT_DIRECTIONAL = 0,
            LIGHT_OMNI        = 1,
            LIGHT_SPOT        = 2
        };

        ~Light() override = default;

        /**
         * Returns the RGB color and the intensity factor
         */
        [[nodiscard]] inline const vec4 &getColorAndIntensity() const { return colorAndIntensity; }

        /**
         * Sets the RGB color and the intensity factor
         */
        inline void setColorAndIntensity(const vec4 colorAndIntensity) { this->colorAndIntensity = colorAndIntensity; }

        /**
         * If `true`, the light will cast real-time shadows.<br>
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.
         */
        [[nodiscard]] inline bool getCastShadows() const { return castShadows; }

        /**
         * Sets to `true` to makes the light cast real-time shadow.<br>
         * This has a significant performance cost. Only enable shadow rendering when it makes a noticeable difference in the scene's appearance.<br>
         * Changing this parameter have no effect after adding the light to the scene (to avoid destroying shadow map renderers during frame rendering),
         * you have to remove the light from the scene, change the setting, then add the light to the scene (adding and removing nodes from the
         * scene if a deferred process).
         */
        void setCastShadows(bool castShadows);

        [[nodiscard]] inline int32_t getLightType() const { return lightType; }

        void setProperty(const string &property, const string &value) override;

    protected:
        explicit Light(const string &nodeName = TypeNames[LIGHT], Type type = LIGHT);

        explicit Light(vec4 color, const string &nodeName = TypeNames[LIGHT], Type type = LIGHT);

    private:
        const LightType lightType{LIGHT_UNKNOWN};
        vec4  colorAndIntensity{1.0f, 1.0f, 1.0f, 1.0f};
        bool  castShadows{false};
    };

}
