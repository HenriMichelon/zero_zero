/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.AnimationPlayer;

import z0.Constants;
import z0.Tools;

namespace z0 {

    AnimationPlayer::AnimationPlayer(const shared_ptr<Node>& node, const string &name) :
              Node{name, ANIMATION_PLAYER}, node{node} {}

    void AnimationPlayer::_update(float _) {
        const auto now = std::chrono::steady_clock::now();
        const auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation && node) {
            for (auto track : animation->getTracks()) {
                auto value = track.interpolate(duration);
                switch (track.type) {
                case AnimationType::TRANSLATION:
                    node->setPosition(get<vec3>(value));
                    break;
                case AnimationType::ROTATION: {
                    node->setRotation(get<quat>(value));
                    break;
                }
                case AnimationType::SCALE:
                    node->setScale(get<vec3>(value));
                    break;
                default:
                }
            }
        }
    }

    void AnimationPlayer::_onEnterScene() {
        Node::_onEnterScene();
        if (autostart) {
            play(currentAnimation);
        }
    }

    void AnimationPlayer::play(const string &name) {
        currentAnimation = name;
        isPlaying = true;
        startTime = chrono::steady_clock::now();
    }

    void AnimationPlayer::stop() {
        isPlaying = false;
    }

    shared_ptr<Animation> AnimationPlayer::getAnimation() {
        if (!libraries.contains(currentLibrary)) { return nullptr; }
        return libraries[currentLibrary]->get(currentAnimation);
    }

}
