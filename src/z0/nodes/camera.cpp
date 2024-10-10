module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

module z0;

import :Constants;
import :Node;
import :Application;
import :Camera;

namespace z0 {

    Camera::Camera(const string &nodeName):
        Node{nodeName, CAMERA} {
        setPerspectiveProjection(fov, nearDistance, farDistance);
        updateViewMatrix();
    }

    void Camera::setOrthographicProjection(const float left, const float right,
                                           const float top, const float  bottom,
                                           const float near, const float far) {
        projectionMatrix       = mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(const float fov, const float near, const float far) {
        this->fov                = fov;
        nearDistance             = near;
        farDistance              = far;
        const auto aspect        = Application::get()._getDevice().getAspectRatio();
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


        /* https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling */
        // const auto camPos = position;
        // const auto camFront = rotationQuaternion * AXIS_FRONT;
        // const auto camUp = rotationQuaternion * AXIS_UP;
        // const auto camRight = rotationQuaternion * AXIS_RIGHT;
        //
        // const auto zNear = nearDistance;
        // const auto zFar = farDistance;
        // const auto halfVSide = zFar * tanf(radians(fov) * .5f);
        // const auto halfHSide = halfVSide * Application::get()._getDevice().getAspectRatio();
        // const auto frontMultFar = zFar * camFront;
        //
        // frustum.nearFace = {camPos + zNear * camFront, camFront};
        // frustum.farFace = {camPos + frontMultFar, -camFront};
        // frustum.rightFace = {camPos, glm::cross(camUp, frontMultFar + camRight * halfHSide)};
        // frustum.leftFace = {camPos, glm::cross(frontMultFar - camRight * halfHSide, camUp)};
        // frustum.topFace = {camPos, glm::cross(frontMultFar + camUp * halfVSide, camRight)};
        // frustum.bottomFace = {camPos, glm::cross(camRight, frontMultFar - camUp * halfVSide)};
    }

    // glm::vec3 getTransformScale(const glm::mat4& transform) {
    //     float sx = glm::length(glm::vec3{transform[0][0], transform[0][1], transform[0][2]});
    //     float sy = glm::length(glm::vec3{transform[1][0], transform[1][1], transform[1][2]});
    //     float sz = glm::length(glm::vec3{transform[2][0], transform[2][1], transform[2][2]});
    //     return {sx, sy, sz};
    // }
    //
    // // todo : calculate bounding sphere for meshinstance
    // // Sphere calculateBoundingSphereWorld(
    // //     const glm::mat4& transform,
    // //     const Sphere& s) {
    // //     const auto scale = getTransformScale(transform);
    // //     float maxScale = std::max({scale.x, scale.y, scale.z});
    // //     auto sphereWorld = s;
    // //     sphereWorld.radius *= maxScale;
    // //     sphereWorld.center = glm::vec3(transform * glm::vec4(sphereWorld.center, 1.f));
    // //     return sphereWorld;
    // // }
    //
    // bool Camera::_isOnFrustum(const Node* node) const {
    //     //Get global scale is computed by doing the magnitude of
    //     //X, Y and Z model matrix's column.
    //     const glm::vec3 globalScale = getTransformScale(node->getTransformGlobal());
    //
    //     //Get our global center with process it with the global model matrix of our transform
    //     const glm::vec3 globalCenter = node->getPositionGlobal();
    //
    //     //To wrap correctly our shape, we need the maximum scale scalar.
    //     const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);
    //
    //     //Max scale is assuming for the diameter. So, we need the half to apply it to our radius
    //     Sphere globalSphere(globalCenter, 50.0f * (maxScale * 0.5f));
    //
    //     //Check Firstly the result that have the most chance
    //     //to failure to avoid to call all functions.
    //     return (globalSphere.isOnOrForwardPlane(frustum.leftFace) &&
    //         globalSphere.isOnOrForwardPlane(frustum.rightFace) &&
    //         globalSphere.isOnOrForwardPlane(frustum.farFace) &&
    //         globalSphere.isOnOrForwardPlane(frustum.nearFace) &&
    //         globalSphere.isOnOrForwardPlane(frustum.topFace) &&
    //         globalSphere.isOnOrForwardPlane(frustum.bottomFace));
    // }

    void Camera::_updateTransform(const mat4 &parentMatrix) {
        Node::_updateTransform(parentMatrix);
        updateViewMatrix();
    }

    void Camera::_updateTransform() {
        Node::_updateTransform();
        updateViewMatrix();
    }

}
