/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <fastgltf/math.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.Animation;

import z0.Constants;
import z0.Tools;

namespace z0 {

    // TODO : no loop
    // TODO AnimationInterpolation
    variant<vec3, quat> Animation::Track::interpolate(const double currentTimeFromStart) const {
        if (!enabled || keyTime.size() < 2) {
            return keyValue[0];
        }
        const auto currentTime = fmod(currentTimeFromStart, static_cast<double>(duration));
        // log("--------------------");
        // log(to_string(currentTime) + " / " + to_string(currentTimeFromStart));

        const auto it = lower_bound(keyTime.begin(), keyTime.end(), static_cast<float>(currentTime));
        auto nextIndex = std::distance(keyTime.begin(), it);
        auto previousIndex = nextIndex > 0 ? nextIndex - 1 : 0;
        // log(to_string(previousIndex) + " / " + to_string(nextIndex));
        bool overflow = nextIndex == keyTime.size();

        const auto& previousTime = nextIndex == 0 ? 0.0f : keyTime[previousIndex];
        const auto nextTime = overflow ? duration : keyTime[nextIndex];
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (nextTime - previousTime));
        // log(to_string(previousTime) + " / " + to_string(nextTime));
        // log(to_string(interpolationValue));

        const auto& previousValue = keyValue[previousIndex];
        const auto nextValue = overflow ? keyValue[0] : keyValue[nextIndex]; // TODO no loop
        switch (type) {
            case AnimationType::TRANSLATION:
            case AnimationType::SCALE:
                // log(to_string(get<vec3>(previousValue)) + " / " + to_string(get<vec3>(nextValue)));
                // log(to_string(lerp(get<vec3>(previousValue), get<vec3>(nextValue), interpolationValue)));
                return glm::lerp(get<vec3>(previousValue), get<vec3>(nextValue), interpolationValue);
            case AnimationType::ROTATION:
                return slerp(get<quat>(previousValue), get<quat>(nextValue), interpolationValue);
            default:
                return keyValue[0];
        }
    }

    Animation::Animation(const string &name): Resource{name} {}

    Animation::Animation(uint32_t tracksCount, const string &name): Resource {name} {
        tracks.resize(tracksCount);
    }

}
