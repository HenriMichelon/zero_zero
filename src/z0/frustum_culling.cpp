module;
/*
 * https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
 */
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Constants;
import :Node;
import :Application;
import :FrustumCulling;

namespace z0 {

    Frustum::Frustum(const Node* node, const float fovY, const float zNear, const float zFar) {
        const auto& position = node->getPositionGlobal();
        const auto front = node->getFrontVector();
        const auto right = node->getRightVector();
        const auto up = node->getUpVector();

        const float halfVSide = zFar * tanf(radians(fovY) * .5f);
        const float halfHSide = halfVSide *  Application::get()._getDevice().getAspectRatio();
        const vec3 frontMultFar = zFar * front;

        nearFace = { position + zNear * front, front };
        farFace = { position + frontMultFar, -front };
        rightFace = { position,
                                glm::cross(frontMultFar - right * halfHSide, up) };
        leftFace = { position,
                                glm::cross(up,frontMultFar + right * halfHSide) };
        topFace = { position,
                                glm::cross(right, frontMultFar - up * halfVSide) };
        bottomFace = { position,
                                glm::cross(frontMultFar + up * halfVSide, right) };
    }

    bool isOnOrForwardPlane(const Plane& plane, const vec3& position) {
        return plane.getSignedDistanceToPlane(position) > -5.0f;
    }

    bool Frustum::isOnFrustum(const Node* node) const {
        const auto center = node->getPositionGlobal();
        return  (isOnOrForwardPlane(farFace, center) && isOnOrForwardPlane(nearFace, center) &&
                 isOnOrForwardPlane(leftFace, center) && isOnOrForwardPlane(rightFace, center) &&
                 isOnOrForwardPlane(topFace, center) && isOnOrForwardPlane(bottomFace, center));
    }



}