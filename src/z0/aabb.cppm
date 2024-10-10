module;
#include "z0/libraries.h"

export module z0:AABB;

import :Mesh;

export namespace z0 {

    /**
    * A 3D axis-aligned bounding box.
    */
    struct AABB {
        vec3 center{ 0.f, 0.f, 0.f };
        vec3 extents{ 0.f, 0.f, 0.f };

        inline AABB(const vec3& min, const vec3& max) :
            center{ (max + min) * 0.5f },
            extents{ max.x - center.x, max.y - center.y, max.z - center.z }
        {}

        inline AABB(const vec3& inCenter, const float iI, const float iJ, const float iK)
            : center{ inCenter }, extents{ iI, iJ, iK }
        {}

        explicit AABB(const Mesh*);

    };
}