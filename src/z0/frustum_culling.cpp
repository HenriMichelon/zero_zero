module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Constants;
import :Node;
import :Application;
import :FrustumCulling;
import :AABB;

namespace z0 {

    /*
     * https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    */

    Frustum::Frustum(const Node* node, const float fovY, const float zNear, const float zFar):
        Frustum(node, node->getPositionGlobal(), fovY, zNear, zFar) {
    }

    Frustum::Frustum(const Node* node, const vec3 position, const float fovY, const float zNear, const float zFar) {
        const auto front = node->getFrontVector();
        const auto right = node->getRightVector();
        const auto up = node->getUpVector();

        const float halfVSide = zFar * tanf(radians(fovY) * .5f);
        const float halfHSide = halfVSide *  Application::get()._getDevice().getAspectRatio();
        const vec3 frontMultFar = zFar * front;

        nearFace = { position + zNear * front, front };
        farFace = { position + frontMultFar, -front };
        rightFace = { position,cross(frontMultFar - right * halfHSide, up) };
        leftFace = { position, cross(up,frontMultFar + right * halfHSide) };
        topFace = { position, cross(right, frontMultFar - up * halfVSide) };
        bottomFace = { position, cross(frontMultFar + up * halfVSide, right) };
    }

    bool isOnOrForwardPlane(const Plane& plane, const vec3& position) {
        return plane.getSignedDistanceToPlane(position) > -.0f;
    }

    bool Frustum::isOnFrustum(const MeshInstance* meshInstance) const {
        const auto & aabb = meshInstance->getAABB();
        vec3 vmin, vmax;
        bool ret = true;
        for (int i = 0; i < 6; ++i) {
            const auto& plane = getPlane(i);
            // X axis
            if (plane.normal.x < 0) {
                vmin.x = aabb.min.x;
                vmax.x = aabb.max.x;
            } else {
                vmin.x = aabb.max.x;
                vmax.x = aabb.min.x;
            }
            // Y axis
            if (plane.normal.y < 0) {
                vmin.y = aabb.min.y;
                vmax.y = aabb.max.y;
            } else {
                vmin.y = aabb.max.y;
                vmax.y = aabb.min.y;
            }
            // Z axis
            if (plane.normal.z < 0) {
                vmin.z = aabb.min.z;
                vmax.z = aabb.max.z;
            } else {
                vmin.z = aabb.max.z;
                vmax.z = aabb.min.z;
            }
            if (plane.getSignedDistanceToPlane(vmin) < 0) {
                return false;
            }
            if (plane.getSignedDistanceToPlane(vmax) <= 0) {
                ret = true;
            }
        }
        return ret;
    }



}