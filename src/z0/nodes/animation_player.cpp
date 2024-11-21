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

    void AnimationPlayer::_update(float alpha) {
        const auto animation = getAnimation();
        if (animation && node) {
            for (auto track : animation->getTracks()) {
                auto value = track.interpolate(alpha);
                switch (track.type) {
                case AnimationType::TRANSLATION:
                    // node->translate(get<vec3>(value));
                    break;
                case AnimationType::ROTATION: {
                    // auto rot = eulerAngles(get<quat>(value));
                    // TODO : implements rotate(quat)
                    node->setRotation(get<quat>(value));
                    break;
                }
                case AnimationType::SCALE:
                    node->setScale(get<vec3>(value));
                    break;
                }
            }
        }
    }

    shared_ptr<Animation> AnimationPlayer::getAnimation() {
        if (!libraries.contains(currentLibrary)) { return nullptr; }
        return libraries[currentLibrary]->get(currentAnimation);
    }

}
