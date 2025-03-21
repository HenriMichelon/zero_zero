/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.nodes.AnimationPlayer;

import z0.Constants;
import z0.Log;
import z0.Tools;

namespace z0 {

    void AnimationPlayer::seek(const float duration) {
        const auto animation = getAnimation();
        for (auto trackIndex = 0; trackIndex < animation->getTracksCount(); trackIndex++) {
            const auto& value = animation->getInterpolatedValue(
                       trackIndex,
                       duration,
                       false);
            currentTracksState[trackIndex] = value.frameTime;
            apply(value);
        }
    }

    void AnimationPlayer::apply(const Animation::TrackKeyValue& value) {
        switch (value.type) {
        case AnimationType::TRANSLATION:
            target->setPosition(value.value + initialPosition);
            break;
        case AnimationType::ROTATION:
            target->setRotation(value.value + initialRotation);
            break;
        case AnimationType::SCALE:
            target->setScale(value.value + initialScale);
            break;
        default:
            die("Unknown animation type");
        }
    }

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
        if (animation && target) {
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
                   apply(value);
                }
            }
        }
    }

    void AnimationPlayer::_onEnterScene() {
        Node::_onEnterScene();
        if (!target && getParent()) {
            setTarget(getParent());
        }
        if (autoStart) {
            play();
        }
    }

    void AnimationPlayer::setTarget(Node *target) {
        this->target = target;
        initialPosition = target->getPosition();
        initialRotation = target->getRotation();
        initialScale = target->getScale();
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

    shared_ptr<Node> AnimationPlayer::duplicateInstance() const {
        return make_shared<AnimationPlayer>(*this);
    }

}
