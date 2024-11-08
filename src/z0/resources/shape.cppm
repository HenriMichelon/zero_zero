/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include "z0/libraries.h"

export module z0.Shape;

import z0.Tools;
import z0.Resource;

export namespace z0 {

    /**
     * Base class for all collision shapes
     */
    class Shape : public Resource {
    protected:
        JPH::ShapeSettings* shapeSettings{nullptr}; // https://jrouwe.github.io/JoltPhysics/index.html#memory-management

        explicit Shape(const string &resName);

    public:
        [[nodiscard]] inline JPH::ShapeSettings *_getShapeSettings() const { return shapeSettings; }
    };

    /**
     * Box shaped collision Shape
     */
    class BoxShape : public Shape {
    public:
        /**
         * Creates a BoxShape with the given extends
         */
        explicit BoxShape(const vec3& extends, const string &resName = "BoxShape");

        shared_ptr<Resource> duplicate()  const override;

    private:
        const vec3 extends;
    };

    /**
     * Sphere shaped collision Shape
     */
    class SphereShape : public Shape {
    public:
        /**
         * Create a SphereShape with the given radius
         */
        explicit SphereShape(float radius, const string &resName = "SphereShape");

    private:
        // const float radius;

        explicit SphereShape(const string &resName) : Shape(resName) {};
    };

}
