module;
#include "z0/modules.h"

export module Z0:MeshInstance;

import :Mesh;
import :Material;
import :Node;

namespace z0 {

    /**
     * Node that hold a Mesh.
     */
    export class MeshInstance: public Node {
    public:
        /**
         * Creates a MeshInstance with the given Mesh
         */
        explicit MeshInstance(const shared_ptr<Mesh>& _mesh, const string& name = "MeshInstance"):
           Node{name},
           mesh{_mesh}  {}

        /**
         * Returns the associated Mesh
         */
        [[nodiscard]] const shared_ptr<Mesh>& getMesh() const { return mesh; }

        /**
         * Returns `true` if the Mesh is valid
         */
        [[nodiscard]] bool isValid() const { return mesh != nullptr; }

        /**
         * Set to `true` to have the Mesh outlined starting to the next frame
         */
        void setOutlined(bool o) { outlined = o; }

        /**
         * Returns `true` if the Mesh will be outlined during the next frame
         */
        [[nodiscard]] bool isOutlined() const { return outlined; }

        /**
         * Sets the outline material. The material **must** belong to the OutlineMaterials collection.
         */
        void setOutlineMaterial(const shared_ptr<ShaderMaterial>& mat) { outlineMaterial = mat; }

        /**
         * Returns the current outlining material
         */
        [[nodiscard]] shared_ptr<ShaderMaterial>& getOutlineMaterial() { return outlineMaterial; }

    protected:
        shared_ptr<Node> duplicateInstance() override  {
             return make_shared<MeshInstance>(*this);
        }

    private:
        bool outlined{false};
        shared_ptr<Mesh> mesh;
        shared_ptr<ShaderMaterial> outlineMaterial;
    };

}