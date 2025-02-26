/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.resources.AnimationLibrary;

import z0.resources.Animation;
import z0.resources.Resource;

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
        explicit inline AnimationLibrary(const string &name = "AnimationLibrary") : Resource{name} {}

        /**
         * Adds the \ref Animation to the library, accessible by the key name.
         */
        inline void add(const string& keyName, const shared_ptr<Animation> &animation) {
            if (animations.empty()) {
                defaultAnimation = keyName;
            }
            animations[keyName] = animation;
        }

        /**
         * Returns the \ref Animation with the key name.
         */
        [[nodiscard]] inline auto get(const string& keyName) const { return animations.at(keyName); }

        /**
         * Returns `true` if the library stores an \ref Animation with name as the key.
         */
        [[nodiscard]] inline auto has(const string& keyName) const { return animations.contains(keyName); }

        /**
         * Returns the name of the default animation
         */
        [[nodiscard]] inline const auto& getDefault() const { return defaultAnimation; }

    private:
        string defaultAnimation;
        map<string, shared_ptr<Animation>> animations;
    };

}
