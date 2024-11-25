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
        if (starting) {
            startTime = chrono::steady_clock::now();
            playing = true;
            starting = false;
        } else if (!playing) {
            return;
        }
        const auto now = chrono::steady_clock::now();
        const auto duration = (chrono::duration_cast<chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation && node) {
            for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
                const auto& value = animation->getInterpolatedValue(
                    trackIndex,
                    duration + lastTracksState[trackIndex]);
                currentTracksState[trackIndex] = value.frameTime;
                if (value.ended) {
                    stop();
                } else {
                    switch (value.type) {
                    case AnimationType::TRANSLATION:
                        // cout << to_string(get<vec3>(value.value)) << endl;
                        node->setPosition(get<vec3>(value.value));
                        break;
                    case AnimationType::ROTATION: {
                        node->setRotation(get<quat>(value.value));
                        break;
                    }
                    case AnimationType::SCALE:
                        node->setScale(get<vec3>(value.value));
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

    void AnimationPlayer::setCurrentLibrary(const string &name) {
        if (libraries.contains(name)) {
            currentLibrary = name;
            setCurrentAnimation(libraries[currentLibrary]->getDefault());

        }
    }


    void AnimationPlayer::setCurrentAnimation(const string &name) {
        if (libraries[currentLibrary]->has(name)) {
            currentAnimation = name;
            if (currentTracksState.size() != getAnimation()->getTracksCount()) {
                currentTracksState.resize(getAnimation()->getTracksCount());
                lastTracksState.resize(getAnimation()->getTracksCount());
                ranges::fill(lastTracksState, 0.0f);
            }
        }
    }


    void AnimationPlayer::play(const string &name) {
        if (playing) { return; }
        if (name.empty()) {
            setCurrentAnimation(libraries[currentLibrary]->getDefault());
        } else {
            setCurrentAnimation(name);
        }
        starting = true;
    }

    void AnimationPlayer::stop(const bool keepState) {
        if (!playing) { return; }
        playing = false;
        if (keepState) {
            lastTracksState = currentTracksState;
        } else {
            ranges::fill(lastTracksState, 0.0f);
        }
    }

    shared_ptr<Animation> AnimationPlayer::getAnimation() {
        return libraries[currentLibrary]->get(currentAnimation);
    }

}
