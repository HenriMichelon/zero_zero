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
        explicit AnimationPlayer(const string &name = TypeNames[ANIMATION_PLAYER]): Node{name, ANIMATION_PLAYER} {};

        AnimationPlayer(const shared_ptr<Node>& node, const string &name = TypeNames[ANIMATION_PLAYER]);

        [[nodiscard]] inline const string& getCurrentLibrary() const { return currentLibrary; }

        [[nodiscard]] inline const string& getCurrentAnimation() const { return currentAnimation; }

        inline void setCurrentLibrary(const string &name) { currentLibrary = name; }

        inline void setCurrentAnimation(const string &name) { currentAnimation = name; }

        inline void add(const string& name, const shared_ptr<AnimationLibrary>& library) { libraries[name] = library; }

        shared_ptr<Animation> getAnimation();

        [[nodiscard]] inline shared_ptr<AnimationLibrary> getLibrary() { return libraries[currentLibrary]; }

        void _update(float alpha) override;

        void _onEnterScene() override;

        void play(const string &name);

        void stop();

    private:
        bool autostart{true};
        bool isPlaying{false};
        chrono::time_point<chrono::steady_clock> startTime;
        shared_ptr<Node> node;
        string currentLibrary{""};
        string currentAnimation{""};
        map<string, shared_ptr<AnimationLibrary>> libraries;
    };

}
