module;
#include "z0/libraries.h"

export module z0:Camera;

import :Constants;
import :Node;

export namespace z0 {

    /**
     * Camera node, displays from a point of view.
     */
    class Camera : public Node {
    public:
        ~Camera() override = default;

        /**
         * Creates a Camera
         */
        explicit Camera(const string &nodeName = "Camera");

        /**
         * Returns `true` if the camera is the currently active camera for the current scene.
         * Use Application::activateCamera() to activate a camera
         */
        [[nodiscard]] inline bool isActive() const { return active; }

        /**
         * Sets the camera projection to orthogonal mode.
         * @param left, right, top, bottom size of the view
         * @param near, far clip planes
         */
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        /**
         * Sets the camera projection to perspective mode.
         * @param fov field of view angle in degrees
         * @param near nearest clip plane
         * @param far farthest clip plane
         */
        void setPerspectiveProjection(float fov, float near, float far);

        /**
         * Returns the projection matrix
         */
        [[nodiscard]] const mat4 &getProjection() const { return projectionMatrix; }

        /**
         * Returns the view matrix
         */
        [[nodiscard]] inline const mat4 &getView() const { return viewMatrix; }

        /**
         * Returns the 2D coordinates in the rendering window that maps to the given 3D point in world space.
         */
        [[nodiscard]] vec2 unproject(vec3 worldCoords);

        /**
         * Returns the camera near clipping distance
         */
        [[nodiscard]] inline float getNearClipDistance() const { return nearDistance; }

        /**
         * Returns the camera far clipping distance
         */
        [[nodiscard]] inline float getFarClipDistance() const { return farDistance; }

        /**
         * Returns the camera FOV in degrees
         */
        [[nodiscard]] inline float getFov() const { return fov; }

    private:
        // Field of view in degrees
        float fov{75.0};
        // Nearest clipping distance
        float nearDistance{0.1f};
        // Furthest clipping distance
        float farDistance{300.0f};
        // Is this the currently active camera ?
        bool active{false};
        // Projection matrix for the global UBO
        mat4 projectionMatrix{1.0f};
        // View matrix for the global UBO
        mat4 viewMatrix{1.0f};

        void updateViewMatrix();

    public:
        inline void _setActive(const bool isActive) { active = isActive; }

        void _updateTransform(const mat4 &parentMatrix) override;

        void _updateTransform() override;
    };

} // namespace z0
