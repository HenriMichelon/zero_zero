module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>

export module Z0:Camera;

import :Constants;
import :Node;
import :Application;

export namespace z0 {

    /**
     * Camera node, displays from a point of view.
     */
    class Camera: public Node {
    public:
        /**
         * Creates a Camera
         */
        explicit Camera(const string& nodeName = "Camera"):
            Node{nodeName} {
            setPerspectiveProjection(fov, nearDistance, farDistance);
            setViewDirection();
        }
        ~Camera() override = default;

        /**
         * Returns `true` if the camera is the currently active camera for the current scene.
         * Use Application::activateCamera() to activate a camera
         */
        [[nodiscard]] bool isActive() const { return active; }

        /**
         * Sets the camera projection to orthogonal mode.
         * @param left, right, top, bottom size of the view
         * @param near, far clip planes
         */
        void setOrthographicProjection(const float left, const float right,
                                       const float top, const float bottom,
                                       const float _near, const float _far) {
            projectionMatrix = mat4{1.0f};
            projectionMatrix[0][0] = 2.f / (right - left);
            projectionMatrix[1][1] = 2.f / (bottom - top);
            projectionMatrix[2][2] = 1.f / (_far - _near);
            projectionMatrix[3][0] = -(right + left) / (right - left);
            projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
            projectionMatrix[3][2] = -_near / (_far - _near);
        }

        /**
         * Sets the camera projection to perspective mode.
         * @param fov field of view angle in degrees
         * @param near nearest clip plane
         * @param far farthest clip plane
         */
        void setPerspectiveProjection(const float _fov, const float _near, const float _far) {
            fov = _fov;
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

        /**
         * Returns the projection matrix
         */
        [[nodiscard]] const mat4& getProjection() {
            if (usePerspectiveProjection) {
                setPerspectiveProjection(fov, nearDistance, farDistance);
            }
            return projectionMatrix;
        }

        /**
         * Returns the view matrix
         */
        [[nodiscard]] inline const mat4& getView() const { return viewMatrix; }

        /**
         * Returns the 2D coordinate in the rendering window that maps to the given 3D point in world space.
         */
        [[nodiscard]] vec2 unproject(const vec3 worldCoords) {
            vec4 clipCoords = getProjection() * getView() * vec4(worldCoords, 1.0f);
            vec3 ndcCoords = vec3(clipCoords) / clipCoords.w;
            return {
                (VECTOR_SCALE.x * (ndcCoords.x + 1.0f) / 2.0f),
                VECTOR_SCALE.y - (VECTOR_SCALE.y * (ndcCoords.y + 1.0f) / 2.0f)
            };
        }

    private:
        float fov{75.0};
        float nearDistance{0.1f};
        float farDistance{100.1f};
        bool usePerspectiveProjection{false};
        mat4 projectionMatrix{1.0f};
        mat4 viewMatrix{1.0f};
        const vec3 direction{0.0f, 0.0f, 1.0f };
        bool active{false};

        void setViewDirection() {
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


    public:
        void _setActive(const bool isActive) { active = isActive; }
        void _updateTransform(const mat4& parentMatrix) override {
            Node::_updateTransform(parentMatrix);
            setViewDirection();
        }

        void _updateTransform() override {
            Node::_updateTransform();
            setViewDirection();
        }
    };

}