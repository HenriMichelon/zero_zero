/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Animation;

import z0.Constants;
import z0.Node;
import z0.Resource;

export namespace z0 {

    /**
     * Holds data that can be used to animate anything.
     */
    class Animation : public Resource {
    public:

        struct Track {
            AnimationType                type;
            AnimationInterpolation       interpolation{AnimationInterpolation::LINEAR};
            bool                         enabled{true};
            float                        duration{0.0f};
            vector<float>                keyTime;
            vector<variant<vec3, quat>>  keyValue;
        };

        struct TrackKeyValue {
            bool                 ended;
            float                frameTime;
            AnimationType        type;
            variant<vec3, quat>  value;
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
        [[nodiscard]] inline AnimationLoopMode getLoopMode() const { return loopMode; }

        /**
         * Returns the number of tracks
         */
        [[nodiscard]] inline size_t getTracksCount() const { return tracks.size(); }

        /**
         * Returns a given track
         */
        [[nodiscard]] inline Track& getTrack(const uint32_t index) { return tracks.at(index); }

        /**
         * Returns the interpolated value at the given time (in seconds, from start of the animation) for a track.
         * @param trackIndex
         * @param currentTimeFromStart
         */
        [[nodiscard]] TrackKeyValue getInterpolatedValue(uint32_t trackIndex, double currentTimeFromStart) const;

    private:
        AnimationLoopMode loopMode{AnimationLoopMode::NONE};
        vector<Track> tracks;
    };

}
