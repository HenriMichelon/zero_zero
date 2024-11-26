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

    void AnimationPlayer::_update(const float alpha) {
        Node::_update(alpha);
        if (starting) {
            startTime = chrono::steady_clock::now();
            playing = true;
            starting = false;
            auto params = Playback{.animationName = currentAnimation};
            emit(on_playback_start, &params);
        } else if (!playing) {
            return;
        }
        const auto now = chrono::steady_clock::now();
        const auto duration = (chrono::duration_cast<chrono::milliseconds>(now - startTime).count()) / 1000.0;
        const auto animation = getAnimation();
        if (animation && parent) {
            // cout << parent->getId() << " / " << animation->getId() << " : " << animation->getName() << endl;
            for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
                const auto& value = animation->getInterpolatedValue(
                    trackIndex,
                    duration + lastTracksState[trackIndex],
                    reverse);
                currentTracksState[trackIndex] = value.frameTime;
                if (value.ended) {
                    stop();
                    auto params = Playback{.animationName = currentAnimation};
                    emit(on_playback_finish, &params);
                } else {
                    switch (value.type) {
                    case AnimationType::TRANSLATION:
                        parent->setPosition(value.value);
                        break;
                    case AnimationType::ROTATION:
                        parent->setRotation(value.value);
                        break;
                    case AnimationType::SCALE:
                        // cout << parent->getName() << " : " << to_string(value.value) << endl;
                        parent->setScale(value.value);
                        break;
                    default:
                        die("Unknown animation type");
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
        reverse = false;
    }

    void AnimationPlayer::playBackwards(const string &name) {
        if (playing) { return; }
        play(name);
        reverse = true;
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

    shared_ptr<Node> AnimationPlayer::duplicateInstance() {
        return make_shared<AnimationPlayer>(*this);
    }

}
