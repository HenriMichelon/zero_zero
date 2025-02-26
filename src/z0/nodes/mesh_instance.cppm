/*
* Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.MeshInstance;

import z0.Tools;
import z0.AABB;

import z0.nodes.Node;

import z0.resources.Material;
import z0.resources.Mesh;

namespace z0 {

    /**
     * Node that hold a Mesh.
     */
    export class MeshInstance : public Node {
    public:
        /**
         * Creates a MeshInstance with the given Mesh
         */
        explicit MeshInstance(const shared_ptr<Mesh> &mesh, const string &name = TypeNames[MESH_INSTANCE]);

        // explicit MeshInstance(const MeshInstance & original);

        /**
         * Returns the associated Mesh
         */
        [[nodiscard]] inline const auto& getMesh() const { return mesh; }

        /**
         * Returns `true` if the Mesh is valid
         */
        [[nodiscard]] inline auto isValid() const { return mesh != nullptr; }

        /**
         * Set to `true` to have the Mesh outlined starting to the next frame
         */
        inline auto setOutlined(const bool o) { outlined = o; }

        /**st
         * Returns `true` if the Mesh will be outlined during the next frame
         */
        [[nodiscard]] inline auto isOutlined() const { return outlined; }

        /**
         * Sets the outline material. The material **must** belong to the OutlineMaterials collection.
         */
        inline auto setOutlineMaterial(const shared_ptr<ShaderMaterial> &material) { outlineMaterial = material; }

        /**
         * Returns the current outlining material
         */
        [[nodiscard]] inline auto& getOutlineMaterial() { return outlineMaterial; }

        /**
         * Returns the world space axis aligned bounding box
         */
        [[nodiscard]] inline const auto& getAABB() const { return worldAABB; }

        friend inline bool operator<(const MeshInstance& a, const MeshInstance& b) {
            return a.mesh < b.mesh;
        }

    protected:
        shared_ptr<Node> duplicateInstance() const override;

    private:
        AABB                       worldAABB;
        bool                       outlined{false};
        shared_ptr<Mesh>           mesh;
        shared_ptr<ShaderMaterial> outlineMaterial;
        
        void _updateTransform(const mat4 &parentMatrix) override; 

        void _updateTransform() override;

    };

}
