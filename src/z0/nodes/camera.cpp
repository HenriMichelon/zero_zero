#include "z0/application.h"
#include "z0/nodes/camera.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cassert>
#include <limits>

namespace z0 {

    Camera::Camera(const string nodeName): Node{std::move(nodeName)} {
        setPerspectiveProjection(fov, nearDistance, farDistance);
        setViewDirection();
    }

    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float _near, float _far) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (_far - _near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -_near / (_far - _near);
    }

    void Camera::setPerspectiveProjection(float _fovy, float _near, float _far) {
        fov = _fovy;
        nearDistance = _near;
        farDistance = _far;
        usePerspectiveProjection = true;
        float aspect = Application::get().getDevice().getAspectRatio();
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(glm::radians(fov) / 2.f);
        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = _far / (_far - _near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(_far * _near) / (_far - _near);
    }

    const glm::mat4& Camera::getProjection() {
        if (usePerspectiveProjection) {
            setPerspectiveProjection(fov, nearDistance, farDistance);
        }
        return projectionMatrix;
    }

    void Camera::updateTransform(const glm::mat4& parentMatrix) {
        Node::updateTransform(parentMatrix);
        setViewDirection();
    }

    void Camera::updateTransform() {
        Node::updateTransform();
        setViewDirection();
    }

    void Camera::setViewDirection() {
        auto rotationQuat = glm::toQuat(glm::mat3(worldTransform));
        auto newDirection = rotationQuat * direction;
        auto position = getPositionGlobal();

        glm::vec3 w{glm::normalize(newDirection)};
        w *= -1;
        const glm::vec3 u{glm::normalize(glm::cross(w, AXIS_UP))};
        const glm::vec3 v{glm::cross(w, u)};

        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }


}