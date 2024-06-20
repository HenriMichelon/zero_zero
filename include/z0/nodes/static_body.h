#pragma once

namespace z0 {

    /**
     * A 3D physics body that can't be moved by external forces. 
     */
    class StaticBody: public PhysicsBody {
    public:
        explicit StaticBody(shared_ptr<Shape> shape,
                            uint32_t layer=1,
                            uint32_t mask=1,
                            const string& name = "StaticBody");
        ~StaticBody() override = default;
    };

}