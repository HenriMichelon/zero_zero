/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.DirectionalLight;

import z0.nodes.Light;

export namespace z0 {

    /**
     * Directional light from a distance, as from the Sun.
     */
    class DirectionalLight : public Light {
    public:
        /**
         * Create a DirectionalLight
         * @param color RGB color and intensity of the light
         * @param nodeName Node name
         */
        explicit DirectionalLight(vec4          color    = {1.0f, 1.0f, 1.0f, 1.0f},
                                  const string &nodeName = TypeNames[DIRECTIONAL_LIGHT]);

        ~DirectionalLight() override = default;

        /**
         * Sets the number of cascades for the shadow map (between 2 and ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS).<br>
         * *must* be called before adding the light to the scene since this value is used when instancing the shadow map
         * renderer for this light.
         */
        void setShadowMapCascadesCount(uint32_t cascadesCount);

        /**
         * Returns the number of cascades for the shadow map
         */
        [[nodiscard]] inline auto getShadowMapCascadesCount() const { return shadowMapCascadesCount; }

        void setProperty(const string &property, const string &value) override;

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    private:
        uint32_t shadowMapCascadesCount{3};
    };

}
