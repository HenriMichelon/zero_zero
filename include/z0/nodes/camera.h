#pragma once

namespace z0 {
    class Camera: public Node {
    public:
        explicit Camera(const string& nodeName = "Camera");
        ~Camera() override = default;

        bool isActive() const { return active; }

        void setOrthographicProjection(float left, float right,
                                       float top, float bottom,
                                       float near, float far);
        void setPerspectiveProjection(float fov,
                                      float near, float far);

        const mat4& getProjection();
        const mat4& getView() const { return viewMatrix; }

        void updateTransform(const mat4& parentMatrix) override;
        void updateTransform() override;

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
    };
}