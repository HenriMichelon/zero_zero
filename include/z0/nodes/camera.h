#pragma once

#include "z0/nodes/node.h"

namespace z0 {
    class Camera: public Node {
    public:
        explicit Camera(const string nodeName = "Camera");
        virtual ~Camera() = default;

        void setOrthographicProjection(float left, float right,
                                       float top, float bottom,
                                       float near, float far);
        void setPerspectiveProjection(float fov,
                                      float near, float far);

        const mat4& getProjection();
        const mat4& getView() const { return viewMatrix; }

        void updateTransform(const mat4& parentMatrix);
        void updateTransform();

    private:
        float fov{75.0};
        float nearDistance{0.1f};
        float farDistance{100.1f};
        bool usePerspectiveProjection{false};
        mat4 projectionMatrix{1.0f};
        mat4 viewMatrix{1.0f};

        const vec3 direction{0.0f, 0.0f, 1.0f };

        void setViewDirection();

    };
}