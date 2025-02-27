/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include <glm/gtx/compatibility.hpp>
#include "z0/libraries.h"

module z0.resources.Animation;

import z0.Constants;
import z0.Log;
import z0.Tools;

namespace z0 {

    Animation::TrackKeyValue Animation::getInterpolatedValue(const uint32_t trackIndex,
                                                             const double currentTimeFromStart,
                                                             const bool reverse) const {
        assert(trackIndex < tracks.size());
        const auto& track = tracks[trackIndex];
        auto value = TrackKeyValue{
            .ended = (!track.enabled ||
                    (loopMode == AnimationLoopMode::NONE && currentTimeFromStart >= track.duration) ||
                    track.keyTime.size() < 2),
            .type = track.type,
        };
        if (value.ended) {
            if (reverse) {
                value.value = track.keyValue[0];
            } else {
                value.value = track.keyValue[track.keyValue.size() - 1];
            }
            return value;
        }

        const auto currentTime = fmod(currentTimeFromStart, static_cast<double>(track.duration));
        value.frameTime = static_cast<float>(currentTime);

        const auto it = ranges::lower_bound(track.keyTime, static_cast<float>(currentTime));
        auto nextIndex = std::distance(track.keyTime.begin(), it);
        if (nextIndex == 0) {
            if (reverse) {
                value.value = track.keyValue[track.keyValue.size() - 1];
            } else {
                value.value = track.keyValue[0];
            }
            return value;
        }

        auto       previousIndex = nextIndex;
        const bool overflow = nextIndex == track.keyTime.size();

        if (reverse) {
            previousIndex = track.keyTime.size() - previousIndex;
            nextIndex = track.keyTime.size() - nextIndex;
        }

        const auto& previousTime = track.keyTime[previousIndex];
        const auto nextTime = overflow ? track.duration : track.keyTime[nextIndex];
        const auto diffTime = nextTime - previousTime;
        const auto interpolationValue = static_cast<float>((currentTime - previousTime) / (diffTime > 0 ? diffTime : 1.0f));

        const auto& previousValue = track.keyValue[previousIndex];
        if (track.interpolation == AnimationInterpolation::LINEAR) {
            const auto nextValue = overflow ? track.keyValue[0] : track.keyValue[nextIndex];
            value.value = lerp(previousValue, nextValue, interpolationValue);
        } else {
            // STEP
            value.value = previousValue;
        }
        return value;
    }

    Animation::Animation(const string &name): Resource{name} {}

    Animation::Animation(const uint32_t tracksCount, const string &name): Resource {name} {
        tracks.resize(tracksCount);
    }

}
