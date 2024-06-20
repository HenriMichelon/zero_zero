#pragma once

namespace z0 {

    /**
     * A 3D physics body that is moved by a physics simulation.
     */
    class RigidBody: public PhysicsBody {
    public:
        explicit RigidBody(shared_ptr<Shape> shape,
                           uint32_t layer=1,
                           uint32_t mask=1,
                           const string& name = "RigidBody");
        ~RigidBody() override = default;

        /**
         * Sets the coefficient of restitution
         * (the ratio of the relative velocity of separation after collision to the relative velocity of approach before collision)
         */
        void setBounce(float value);

    };

}