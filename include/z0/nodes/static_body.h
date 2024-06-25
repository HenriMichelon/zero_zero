#pragma once

namespace z0 {

    /**
     * A 3D physics body that can't be moved by external forces. 
     */
    class StaticBody: public PhysicsBody {
    public:
        /**
         * Creates a StaticBody with a given collision `shape`, 
         * belonging to the `layer` layers and detecting collisions 
         * with bodies having a layer in the `mask` value.
         */
        explicit StaticBody(shared_ptr<Shape> shape,
                            uint32_t layer=0xff,
                            uint32_t mask=0xff,
                            const string& name = "StaticBody");
        ~StaticBody() override = default;
    };

}