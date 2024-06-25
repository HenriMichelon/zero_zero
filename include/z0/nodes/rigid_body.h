#pragma once

namespace z0 {

    /**
     * A 3D physics body that is moved by a physics simulation.
     */
    class RigidBody: public PhysicsBody {
    public:
        /**
         * Creates a RigidBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit RigidBody(shared_ptr<Shape> shape,
                           uint32_t layer=0xff,
                           uint32_t mask=0xff,
                           const string& name = "RigidBody");
        ~RigidBody() override = default;

        /**
         * Sets the coefficient of restitution
         * (the ratio of the relative velocity of separation after collision to the relative velocity of approach before collision)
         */
        void setBounce(float value);

    };

}