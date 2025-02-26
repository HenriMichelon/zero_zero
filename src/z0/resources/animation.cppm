/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.resources.Animation;

import z0.Constants;

import z0.nodes.Node;

import z0.resources.Resource;

export namespace z0 {

    /**
     * Holds data that can be used to animate anything.
     */
    class Animation : public Resource {
    public:
        /**
         * An animation track
         */
        struct Track {
            AnimationType           type;
            AnimationInterpolation  interpolation{AnimationInterpolation::LINEAR};
            bool                    enabled{true};
            float                   duration{0.0f};
            vector<float>           keyTime;
            vector<vec3>            keyValue;
        };

        /**
         * Values returned from getInterpolatedValue()
         */
        struct TrackKeyValue {
            //! `true` if we reach the end of the track
            bool           ended;
            //! corresponding time from the start of the track
            float          frameTime;
            //! animation type
            AnimationType  type;
            //! interpolated value
            vec3           value;
        };

        /**
         * Creates an Animation
         * @param name resource name.
         */
        explicit Animation(const string &name);

        /**
         * Creates an Animation
         * @param tracksCount number of tracks to allocate
         * @param name resource name.
         */
        explicit Animation(uint32_t tracksCount, const string &name);

        /**
         * Sets the looping mode
         */
        inline void setLoopMode(const AnimationLoopMode mode) { loopMode = mode; }

        /**
         * Returns the looping mode
         */
        [[nodiscard]] inline auto getLoopMode() const { return loopMode; }

        /**
         * Returns the number of tracks
         */
        [[nodiscard]] inline auto getTracksCount() const { return tracks.size(); }

        /**
         * Returns a given track
         */
        [[nodiscard]] inline auto& getTrack(const uint32_t index) { return tracks.at(index); }

        /**
         * Returns the interpolated value at the given time (in seconds, from start of the animation) for a track.
         */
        [[nodiscard]] TrackKeyValue getInterpolatedValue(uint32_t trackIndex, double currentTimeFromStart, bool reverse=false) const;

    private:
        AnimationLoopMode loopMode{AnimationLoopMode::NONE};
        vector<Track> tracks;
    };

}
