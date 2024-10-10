module;
#include "z0/libraries.h"

export module z0:FrustumCulling;

import :Constants;
import :Node;
import :MeshInstance;

export namespace z0 {

    struct Plane {
        // normal vector
        vec3 normal{0.f, 1.f, 0.f};
        // distance from the origin to the nearest point in the plane
        float distance{0.f};

        Plane() = default;
        inline Plane(const vec3& p1, const vec3& norm) : normal(normalize(norm)), distance(dot(normal, p1)){}

        inline float getSignedDistanceToPlane(const vec3& point) const { return dot(normal, point) - distance; }
    };

    struct Frustum {
        Plane farFace;
        Plane nearFace;
        Plane leftFace;
        Plane rightFace;
        Plane topFace;
        Plane bottomFace;

        // inline const Plane& getPlane(const int i) const
        // {
        //     switch (i) {
        //     case 0:
        //         return farFace;
        //     case 1:
        //         return nearFace;
        //     case 2:
        //         return leftFace;
        //     case 3:
        //         return rightFace;
        //     case 4:
        //         return topFace;
        //     case 5:
        //         return bottomFace;
        //     default:
        //         return nearFace;
        //     }
        // }

        Frustum(const Node*, float fovY, float zNear, float zFar);

        bool isOnFrustum(const MeshInstance* meshInstance) const;

    };

}