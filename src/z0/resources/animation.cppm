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
            bool                         enabled{true};
            vector<float>                keyTime;
            vector<variant<vec3, quat>>  keyValue;

            variant<vec3, quat> interpolate(double alpha);
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

        [[nodiscard]] inline vector<Track>& getTracks() { return tracks; }

        [[nodiscard]] inline Track& getTrack(const uint32_t index) { return tracks.at(index); }

    private:
        vector<Track> tracks;
    };

}
