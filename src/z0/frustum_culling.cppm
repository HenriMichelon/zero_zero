/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.FrustumCulling;

import z0.Constants;

import z0.nodes.MeshInstance;
import z0.nodes.Node;

export namespace z0 {

    /**
     * %A camera or light frustum
     */
    struct Frustum {
        /**
         * One plane of a frustum cube
         */
        struct Plane {
            //! normal vector
            vec3 normal{0.f, 1.f, 0.f};
            //! distance from the origin to the nearest point in the plane
            float distance{0.f};

            Plane() = default;
            inline Plane(const vec3& p1, const vec3& norm) : normal(normalize(norm)), distance(dot(normal, p1)){}

            inline auto getSignedDistanceToPlane(const vec3& point) const { return dot(normal, point) - distance; }
        };

        //! Far plane
        Plane farFace;
        //! Near plane
        Plane nearFace;
        //! Left plane
        Plane leftFace;
        //! Right plane
        Plane rightFace;
        //! Top plane
        Plane topFace;
        //! Bottom plane
        Plane bottomFace;

        inline const auto& getPlane(const int i) const {
            switch (i) {
            case 0:
                return farFace;
            case 1:
                return nearFace;
            case 2:
                return leftFace;
            case 3:
                return rightFace;
            case 4:
                return topFace;
            case 5:
                return bottomFace;
            default:
                return nearFace;
            }
        }

        Frustum() = default;

        /**
         * Creates a frustum
         * \param node Camera or Light
         * \param fovY Field of view in degrees
         * \param zNear Near clipping distance
         * \param zFar Far clipping distance
         */
        Frustum(const shared_ptr<Node>&node, float fovY, float zNear, float zFar);

        /**
         * Creates a frustum
         * \param node Camera or Light
         * \param position Node position to use instead of the real node position
         * \param fovY Field of view in degrees
         * \param zNear Near clipping distance
         * \param zFar Far clipping distance
         */
        Frustum(const shared_ptr<Node>&node, const vec3& position, float fovY, float zNear, float zFar);

        /**
         * Creates a frustum
         * \param position Node position
         * \param front Front vector
         * \param right Right vector
         * \param up Up vector
         * \param fovY Field of view in degrees
         * \param zNear Near clipping distance
         * \param zFar Far clipping distance
         */
        Frustum(const vec3& position, const vec3& front, const vec3& right, const vec3&up, float fovY, float zNear, float zFar);

        /**
         * Returns `true` if the MeshInstance is in the frustum
         */
        bool isOnFrustum(const shared_ptr<MeshInstance>& meshInstance) const;

    };

}