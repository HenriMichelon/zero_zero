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

export namespace z0 {

    /**
     * A node used for animation playback.
     */
    class AnimationPlayer : public Node {
    public:
        /**
         * Creates an AnimationLibrary
         * @param name resource name.
         */
        explicit AnimationPlayer(const string &name = TypeNames[ANIMATION_PLAYER]): Node{name, ANIMATION_PLAYER} {};

        /**
         * Creates an AnimationLibrary
         * @param node attached node
         * @param name resource name.
         */
        explicit AnimationPlayer(const shared_ptr<Node>& node, const string &name = TypeNames[ANIMATION_PLAYER]);

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
        inline void setCurrentLibrary(const string &name) { currentLibrary = name; }

        /**
         * Sets the current animation name (does not start the animation, only useful with auto-starting)
         */
        inline void setCurrentAnimation(const string &name) { currentAnimation = name; }

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
         * Return the attached node, if any
         */
        [[nodiscard]] inline const shared_ptr<Node>& getNode() const { return node; }

        /**
         * Attach a node to the player
         */
        inline void setNode(const shared_ptr<Node>& node) { this->node = node; }

        /**
         * Starts an animation by its name
         */
        void play(const string &name = "");

        /**
         * Stops the currently playing animation
         */
        void stop(bool keepState = false);

        /**
         * Returns `true` if the animation is currently playing
         */
        inline bool isPlaying() const { return playing; }

        /**
         * Sets the auto start property. Animation are started when entering the node tree.
         */
        inline void setAutoStart(const bool autoStart) { this->autoStart = autoStart; }

        void _update(float alpha) override;

        void _onEnterScene() override;

    private:
        bool autoStart{false};
        bool playing{false};
        chrono::time_point<chrono::steady_clock> startTime;
        double stoppedAt{0.0};
        shared_ptr<Node> node;
        string currentLibrary;
        string currentAnimation;
        map<string, shared_ptr<AnimationLibrary>> libraries;
    };

}
