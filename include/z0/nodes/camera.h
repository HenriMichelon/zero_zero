#pragma once

namespace z0 {

    /**
     * Camera node, displays from a point of view.
     */
    class Camera: public Node {
    public:
        /**
         * Creates a Camera
         */
        explicit Camera(const string& nodeName = "Camera");
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
        void setOrthographicProjection(float left, float right,
                                       float top, float bottom,
                                       float near, float far);

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
        [[nodiscard]] const mat4& getProjection();

        /**
         * Returns the view matrix
         */
        [[nodiscard]] inline const mat4& getView() const { return viewMatrix; }

        /**
         * Returns the 2D coordinate in the rendering window that maps to the given 3D point in world space.
         */
        [[nodiscard]] vec2 unproject(vec3 worldCoords);

    private:
        float fov{75.0};
        float nearDistance{0.1f};
        float farDistance{100.1f};
        bool usePerspectiveProjection{false};
        mat4 projectionMatrix{1.0f};
        mat4 viewMatrix{1.0f};
        const vec3 direction{0.0f, 0.0f, 1.0f };
        bool active{false};

        void setViewDirection();

    public:
        void _setActive(bool isActive) { active = isActive; }
        void _updateTransform(const mat4& parentMatrix) override;
        void _updateTransform() override;
    };
}