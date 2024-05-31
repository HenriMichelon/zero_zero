#pragma once

namespace z0 {

    class RayCast : public Node {
    public:
        RayCast(const string& name);

    private:
        vec3 target{0.0};

    public:
        void _physicsUpdate() override;
    };

}
