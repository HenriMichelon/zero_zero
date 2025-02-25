/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <glm/gtx/quaternion.hpp>
#include "z0/libraries.h"

module z0.nodes.Camera;

import z0.Application;
import z0.Constants;

import z0.nodes.Node;

namespace z0 {

    Camera::Camera(const string &nodeName):
        Node{nodeName, CAMERA} {
        setPerspectiveProjection(fov, nearDistance, farDistance);
        updateViewMatrix();
    }

    void Camera::setNearDistance(const float distance) {
        nearDistance = distance;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setFarDistance(const float distance) {
        farDistance = distance;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setFov(const float fov) {
        this->fov = fov;
        if (perspectiveProjection) setPerspectiveProjection(fov, nearDistance, farDistance);
    }

    void Camera::setOrthographicProjection(const float left, const float right,
                                           const float top, const float  bottom,
                                           const float near, const float far) {
        perspectiveProjection  = false;
        projectionMatrix       = mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(const float fov, const float near, const float far) {
        perspectiveProjection    = true;
        this->fov                = fov;
        nearDistance             = near;
        farDistance              = far;
        const auto aspect      = app().getAspectRatio();
        const auto tanHalfFovy = tan(radians(fov) / 2.f);
        projectionMatrix       = mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    vec2 Camera::unproject(const vec3 worldCoords) {
        const vec4 clipCoords = getProjection() * getView() * vec4(worldCoords, 1.0f);
        const vec3 ndcCoords  = vec3(clipCoords) / clipCoords.w;
        return {
                (VECTOR_SCALE.x * (ndcCoords.x + 1.0f) / 2.0f),
                VECTOR_SCALE.y - (VECTOR_SCALE.y * (ndcCoords.y + 1.0f) / 2.0f)
        };
    }

    void Camera::updateViewMatrix() {
        const auto  rotationQuaternion = toQuat(mat3(worldTransform));
        const auto  newDirection       = rotationQuaternion * AXIS_FRONT;
        const auto &position           = getPositionGlobal();

        const auto w{normalize(newDirection)};
        const auto u{normalize(cross(w, AXIS_UP))};
        const auto v{cross(w, u)};

        viewMatrix       = mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -dot(u, position);
        viewMatrix[3][1] = -dot(v, position);
        viewMatrix[3][2] = -dot(w, position);
    }

    void Camera::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        updateViewMatrix();
    }

    void Camera::_updateTransform() {
        Node::_updateTransform();
        updateViewMatrix();
    }

    shared_ptr<Node> Camera::duplicateInstance() const {
        return make_shared<Camera>(*this);
    }

}
