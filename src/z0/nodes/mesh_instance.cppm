module;
#include "z0/libraries.h"

export module z0:MeshInstance;

import :Mesh;
import :Material;
import :Node;

namespace z0 {

    /**
     * Node that hold a Mesh.
     */
    export class MeshInstance : public Node {
    public:
        /**
         * Creates a MeshInstance with the given Mesh
         */
        explicit MeshInstance(const shared_ptr<Mesh> &mesh, const string &name = "MeshInstance");

        /**
         * Returns the associated Mesh
         */
        [[nodiscard]] inline const shared_ptr<Mesh> &getMesh() const { return mesh; }

        /**
         * Returns `true` if the Mesh is valid
         */
        [[nodiscard]] inline bool isValid() const { return mesh != nullptr; }

        /**
         * Set to `true` to have the Mesh outlined starting to the next frame
         */
        inline void setOutlined(const bool o) { outlined = o; }

        /**st
         * Returns `true` if the Mesh will be outlined during the next frame
         */
        [[nodiscard]] inline bool isOutlined() const { return outlined; }

        /**
         * Sets the outline material. The material **must** belong to the OutlineMaterials collection.
         */
        inline void setOutlineMaterial(const shared_ptr<ShaderMaterial> &material) { outlineMaterial = material; }

        /**
         * Returns the current outlining material
         */
        [[nodiscard]] inline shared_ptr<ShaderMaterial> &getOutlineMaterial() { return outlineMaterial; }

    protected:
        shared_ptr<Node> duplicateInstance() override;

    private:
        bool                       outlined{false};
        shared_ptr<Mesh>           mesh;
        shared_ptr<ShaderMaterial> outlineMaterial;
    };

}
