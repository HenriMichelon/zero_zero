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

    void AnimationPlayer::_update(const float alpha) {
        Node::_update(alpha);
        if (!playing) { return; }
        const auto now = chrono::steady_clock::now();
        const auto duration = (chrono::duration_cast<chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation && node) {
            for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
                const auto& value = animation->getInterpolatedValue(trackIndex, duration);
                if (value.ended) {
                    stop();
                } else {
                    switch (value.type) {
                    case AnimationType::TRANSLATION:
                        // cout << get<vec3>(value.value) << endl;
                        node->setPosition(get<vec3>(value.value));
                        break;
                    case AnimationType::ROTATION: {
                        // node->setRotation(get<quat>(value.value));
                        break;
                    }
                    case AnimationType::SCALE:
                        // node->setScale(get<vec3>(value.value));
                        break;
                    default:
                    }
                }
            }
        }
    }

    void AnimationPlayer::_onEnterScene() {
        Node::_onEnterScene();
        if (autoStart) {
            play();
        }
    }

    void AnimationPlayer::play(const string &name) {
        if (playing) { return; }
        if (!name.empty()) {
            currentAnimation = name;
        }
        startTime = chrono::steady_clock::now();
        playing = true;
    }

    void AnimationPlayer::stop(const bool keepState) {
        if (!playing) { return; }
        playing = false;
        if (keepState) {
            const auto now = chrono::steady_clock::now();
            stoppedAt = chrono::duration_cast<chrono::milliseconds>(now - startTime).count() / 1000.0;
        } else {
            stoppedAt = 0.0;
        }
    }

    shared_ptr<Animation> AnimationPlayer::getAnimation() {
        if (!libraries.contains(currentLibrary)) { return nullptr; }
        return libraries[currentLibrary]->get(currentAnimation);
    }

}
