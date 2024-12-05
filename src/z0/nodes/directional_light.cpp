/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

module z0.nodes.DirectionalLight;

import z0.nodes.Node;

import z0.ShadowMapFrameBuffer;

namespace z0 {

    DirectionalLight::DirectionalLight(const vec4    color,
                                       const string &nodeName):
        Light{color, nodeName, DIRECTIONAL_LIGHT}
    { }

    void DirectionalLight::setShadowMapCascadesCount(const uint32_t cascadesCount) {
        shadowMapCascadesCount = std::max(static_cast<uint32_t>(2), std::min(cascadesCount, ShadowMapFrameBuffer::CASCADED_SHADOWMAP_MAX_LAYERS));
    }

    void DirectionalLight::setProperty(const string &property, const string &value) {
        Light::setProperty(property, value);
        if (property == "shadow_map_cascade_count") {
            setShadowMapCascadesCount(stoi(value));
        }
    }

    shared_ptr<Node> DirectionalLight::duplicateInstance() {
        return make_shared<DirectionalLight>(*this);
    }

}
