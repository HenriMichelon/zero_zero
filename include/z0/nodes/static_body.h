#pragma once

namespace z0 {

    /**
     * A 3D physics body that can't be moved by external forces. 
     */
    class StaticBody: public PhysicsBody {
    public:
        /**
         * Creates a StaticBody with a given collision `shape`, 
         * belonging to the `layer` layers.
         */
        explicit StaticBody(shared_ptr<Shape> shape,
                            uint32_t layer=0xff,
                            const string& name = "StaticBody");
        /**
         * Creates a StaticBody without a collision `shape`, 
         * belonging to the `layer` layers
         */
        explicit StaticBody(uint32_t layer=0xff,
                            const string& name = "StaticBody");
        ~StaticBody() override = default;
    };

}