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

    // TODO AnimationInterpolation
    Animation::TrackKeyValue Animation::getInterpolatedValue(const uint32_t trackIndex, const double currentTimeFromStart) const {
        assert(trackIndex < tracks.size());
        const auto& track = tracks[trackIndex];
        auto value = TrackKeyValue{
            .ended = (!track.enabled ||
                    (loopMode == AnimationLoopMode::NONE && currentTimeFromStart > track.duration) ||
                    track.keyTime.size() < 2),
            .type = track.type,
        };
        if (value.ended)  { return value; }

        const auto currentTime = fmod(currentTimeFromStart, static_cast<double>(track.duration));
        value.frameTime = static_cast<float>(currentTime);
        // log("--------------------");
        // log(to_string(currentTime) + " / " + to_string(currentTimeFromStart));

        const auto it = lower_bound(track.keyTime.begin(), track.keyTime.end(), static_cast<float>(currentTime));
        const auto nextIndex = std::distance(track.keyTime.begin(), it);
        if (nextIndex == 0) {
            value.value = track.keyValue[0];;
            return value;
        }

        const auto previousIndex =nextIndex;
        // log(to_string(previousIndex) + " / " + to_string(nextIndex));
        bool overflow = nextIndex == track.keyTime.size();

        const auto& previousTime = track.keyTime[previousIndex];
        const auto nextTime = overflow ? track.duration : track.keyTime[nextIndex];
        const auto diffTime = nextTime - previousTime;
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (diffTime > 0 ? diffTime : 1.0f));
        // log(to_string(previousTime) + " / " + to_string(nextTime));
        // log(to_string(interpolationValue));

        const auto& previousValue = track.keyValue[previousIndex];
        if (track.interpolation == AnimationInterpolation::LINEAR) {
            const auto nextValue = overflow ? track.keyValue[0] : track.keyValue[nextIndex]; // TODO no loop
            switch (track.type) {
                case AnimationType::TRANSLATION:
                case AnimationType::SCALE:
                    value.value = lerp(get<vec3>(previousValue), get<vec3>(nextValue), interpolationValue);
                    break;
                case AnimationType::ROTATION:
                    value.value = slerp(get<quat>(previousValue), get<quat>(nextValue), interpolationValue);
                    break;
                default:
                    value.ended = true;
            }
        } else {
            // STEP
            value.value = previousValue;
        }
        return value;
    }

    Animation::Animation(const string &name): Resource{name} {}

    Animation::Animation(uint32_t tracksCount, const string &name): Resource {name} {
        tracks.resize(tracksCount);
    }

}
