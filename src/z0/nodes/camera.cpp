#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/nodes/camera.h"
#endif


namespace z0 {

    Camera::Camera(const string& nodeName):
        Node{nodeName} {
        setPerspectiveProjection(fov, nearDistance, farDistance);
        setViewDirection();
    }

    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float _near, float _far) {
        projectionMatrix = mat4{1.0f};
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
        const auto aspect = Application::get()._getDevice().getAspectRatio();
        assert(glm::abs(aspect - numeric_limits<float>::epsilon()) > 0.0f);
        const auto tanHalfFovy = tan(radians(fov) / 2.f);
        projectionMatrix = mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = _far / (_far - _near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(_far * _near) / (_far - _near);
    }

    const mat4& Camera::getProjection() {
        if (usePerspectiveProjection) {
            setPerspectiveProjection(fov, nearDistance, farDistance);
        }
        return projectionMatrix;
    }

    void Camera::updateTransform(const mat4& parentMatrix) {
        Node::updateTransform(parentMatrix);
        setViewDirection();
    }

    void Camera::updateTransform() {
        Node::updateTransform();
        setViewDirection();
    }

    void Camera::setViewDirection() {
        const auto rotationQuat = toQuat(mat3(worldTransform));
        const auto newDirection = rotationQuat * direction;
        const auto position = getPositionGlobal();

        auto w{normalize(newDirection)};
        w *= -1;
        const auto u{normalize(cross(w, AXIS_UP))};
        const auto v{cross(w, u)};

        viewMatrix = mat4{1.f};
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


}