/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.AnimationLibrary;

import z0.Animation;
import z0.Resource;

export namespace z0 {

    /**
     * Container for \ref Animation resources.
     */
    class AnimationLibrary : public Resource {
    public:
        /**
         * Creates an AnimationLibrary
         * @param name resource name.
         */
        explicit inline AnimationLibrary(const string &name) : Resource{name} {}

        inline void add(const string& name, shared_ptr<Animation> animation) { animations[name] = animation; }

        [[nodiscard]] inline shared_ptr<Animation> get(const string& name) const { return animations.at(name); }

        [[nodiscard]] inline bool has(const string& name) const { return animations.contains(name); }

    private:
        map<string, shared_ptr<Animation>> animations;
    };

}
