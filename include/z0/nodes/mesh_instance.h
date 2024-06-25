#pragma once

namespace z0 {

    /**
     * Node that hold a Mesh.
     */
    class MeshInstance: public Node {
    public:
        /**
         * Creates a MeshInstance with the given Mesh
         */
        explicit MeshInstance(const shared_ptr<Mesh>& mesh, const string& name = "MeshInstance");

        /**
         * Returns the associated Mesh
         */
        const shared_ptr<Mesh>& getMesh() const { return mesh; }

        /**
         * Returns `true` if the Mesh is valid
         */
        bool isValid() const { return mesh != nullptr; }

        /**
         * Set to `true` to have the Mesh outlined starting to the next frame
         */
        void setOutlined(bool o) { outlined = o; }

        /**
         * Returns `true` if the Mesh will be outlined during the next frame
         */
        bool isOutlined() const { return outlined; }

        /**
         * Sets the outline material. The material **must** belong to the OutlineMaterials collection.
         */
        void setOutlineMaterial(const shared_ptr<ShaderMaterial>& mat) { outlineMaterial = mat; }
        
        /**
         * Returns the current outlining material
         */
        shared_ptr<ShaderMaterial>& getOutlineMaterial() { return outlineMaterial; }

    protected:
        shared_ptr<Node> duplicateInstance() override;

    private:
        bool outlined{false};
        shared_ptr<Mesh> mesh;
        shared_ptr<ShaderMaterial> outlineMaterial;
    };

}