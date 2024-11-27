/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.AnimationPlayer;

import z0.Animation;
import z0.AnimationLibrary;
import z0.Node;
import z0.Signal;

export namespace z0 {

    /**
     * %A node used for animation playback.
     */
    class AnimationPlayer : public Node {
    public:
        /**
         * Signal playback parameters
         */
        struct Playback : Signal::Parameters {
            //! The animation name
            string animationName;
        };
        //! Signal emitted when an animation began playing
        static constexpr Signal::signal on_playback_start  = "on_playback_start";

        //! Signal emitted when an animation stop playing
        static constexpr Signal::signal on_playback_finish = "on_playback_finish";

        /**
         * Creates an AnimationLibrary
         * @param name resource name.
         */
        explicit AnimationPlayer(const string &name = TypeNames[ANIMATION_PLAYER]): Node{name, ANIMATION_PLAYER} {}

        /**
         * Returns the current library name
         */
        [[nodiscard]] inline const string& getCurrentLibrary() const { return currentLibrary; }

        /**
         * Returns the current animation name
         */
        [[nodiscard]] inline const string& getCurrentAnimation() const { return currentAnimation; }

        /**
         * Sets the current library name
         */
        void setCurrentLibrary(const string &name);

        /**
         * Sets the current animation name (does not start the animation, only useful with auto-starting)
         */
        void setCurrentAnimation(const string &name);

        /**
         * Adds a library accessible by the name.
         */
        inline void add(const string& name, const shared_ptr<AnimationLibrary>& library) { libraries[name] = library; }

        /**
         * Returns the current animation, if any
         */
        shared_ptr<Animation> getAnimation();

        /**
         * Returns the current animation library, if any
         */
        [[nodiscard]] inline shared_ptr<AnimationLibrary> getLibrary() { return libraries[currentLibrary]; }

        /**
         * Starts an animation by its name
         */
        void play(const string &name = "");

        /**
         * Starts an animation by its name, playing it backwards
         */
        void playBackwards(const string &name = "");

        /**
         * Stops the currently playing animation
         */
        void stop(bool keepState = false);

        /**
         * Returns `true` if the animation is currently playing
         */
        inline bool isPlaying() const { return playing; }

        /**
         * Sets the auto start property.
         */
        inline void setAutoStart(const bool autoStart) { this->autoStart = autoStart; }

        void _update(float alpha) override;

        void _onEnterScene() override;

    protected:
        shared_ptr<Node> duplicateInstance() override;

    private:
        bool autoStart{false};
        bool playing{false};
        bool starting{false};
        bool reverse{false};
        chrono::time_point<chrono::steady_clock> startTime;
        // shared_ptr<Node> node;
        string currentLibrary;
        string currentAnimation;
        vector<float> currentTracksState;
        vector<float> lastTracksState;
        map<string, shared_ptr<AnimationLibrary>> libraries;
    };

}
