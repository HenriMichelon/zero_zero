/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.AnimationPlayer;

import z0.Signal;

import z0.nodes.Node;

import z0.resources.Animation;
import z0.resources.AnimationLibrary;

export namespace z0 {

    /**
     * %A node used for animation playback.
     */
    class AnimationPlayer : public Node {
    public:
        /**
         * Signal playback parameters
         */
        struct Playback {
            //! The animation name
            string animationName;
        };
        //! Signal emitted when an animation began playing
        static inline const Signal::signal on_playback_start  = "on_playback_start";

        //! Signal emitted when an animation stop playing
        static inline const Signal::signal on_playback_finish = "on_playback_finish";

        /**
         * Creates an AnimationLibrary
         * @param name resource name.
         */
        explicit AnimationPlayer(const string &name = TypeNames[ANIMATION_PLAYER]): Node{name, ANIMATION_PLAYER} {}

        /**
         * Returns the current library name
         */
        [[nodiscard]] inline const auto& getCurrentLibrary() const { return currentLibrary; }

        /**
         * Returns the current animation name
         */
        [[nodiscard]] inline const auto& getCurrentAnimation() const { return currentAnimation; }

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
        inline auto add(const string& name, const shared_ptr<AnimationLibrary>& library) { libraries[name] = library; }

        /**
         * Returns the current animation, if any
         */
        [[nodiscard]] shared_ptr<Animation> getAnimation();

        /**
         * Returns the current animation library, if any
         */
        [[nodiscard]] inline auto getLibrary() { return libraries[currentLibrary]; }

        /**
         * Starts an animation by its name
         */
        void play(const string &name = "");

        /**
         *
         * Seeks the animation to the seconds point in time (in seconds).
         */
        void seek(float duration);

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
        [[nodiscard]] inline auto isPlaying() const { return playing; }

        /**
         * Sets the auto start property.
         */
        inline auto setAutoStart(const bool autoStart) { this->autoStart = autoStart; }

        /**
         * Sets the node target on which to apply animations
         */
        inline auto setTarget(Node &target) { setTarget(&target); }

        /**
         * Sets the node target on which to apply animations
         */
        void setTarget(Node *target);

        void _update(float alpha) override;

        void _onEnterScene() override;

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    private:
        bool autoStart{false};
        bool playing{false};
        bool starting{false};
        bool reverse{false};
        vec3 initialPosition{0.0f};
        vec3 initialRotation{0.0f};
        vec3 initialScale{1.0f};
        chrono::time_point<chrono::steady_clock> startTime;
        Node* target{nullptr};
        string currentLibrary;
        string currentAnimation;
        vector<float> currentTracksState;
        vector<float> lastTracksState;
        map<string, shared_ptr<AnimationLibrary>> libraries;

        void apply(const Animation::TrackKeyValue&);
    };

}
