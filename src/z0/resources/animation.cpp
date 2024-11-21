/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/math.hpp>
#include <glm/gtx/quaternion.hpp> // for slerp(quat)
#include "z0/libraries.h"

module z0.Animation;

import z0.Constants;
import z0.Tools;

namespace z0 {

    variant<vec3, quat> Animation::Track::interpolate(const double alpha) {
        if (!enabled || keyTime.size() < 2) {
            return keyValue[0];
        }

        // Find the current and next keyframe indices
        auto it = lower_bound(keyTime.begin(), keyTime.end(), static_cast<float>(alpha));
        size_t nextIndex = std::distance(keyTime.begin(), it);
        size_t currentIndex = nextIndex > 0 ? nextIndex - 1 : 0;

        if (nextIndex >= keyTime.size()) {
            nextIndex = keyTime.size() - 1;
        }

        // Compute normalized time between current and next keyframes
        float t1 = keyTime[currentIndex];
        float t2 = keyTime[nextIndex];
        float localAlpha = (static_cast<float>(alpha) - t1) / (t2 - t1);

        // Interpolate based on type
        const auto& currentValue = keyValue[currentIndex];
        const auto& nextValue = keyValue[nextIndex];
        switch (type) {
        case AnimationType::TRANSLATION:
        case AnimationType::SCALE:
            return lerp(get<vec3>(currentValue), get<vec3>(nextValue), localAlpha);
        case AnimationType::ROTATION:
            return currentValue;
            // return slerp(get<quat>(currentValue), get<quat>(nextValue), localAlpha);
        default:
            die("Unknown animation type");
        }
    }

    Animation::Animation(const string &name): Resource{name} {}

    Animation::Animation(uint32_t tracksCount, const string &name): Resource {name} {
        tracks.resize(tracksCount);
    }

}
