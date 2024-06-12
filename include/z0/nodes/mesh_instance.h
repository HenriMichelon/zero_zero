#pragma once

namespace z0 {

    class MeshInstance: public Node {
    public:
        explicit MeshInstance(const shared_ptr<Mesh>& _mesh, const string& name = "MeshInstance");

        const shared_ptr<Mesh>& getMesh() const { return mesh; }
        bool isValid() const { return mesh != nullptr; }
        void setOutlined(bool o) { outlined = o; }
        bool isOutlined() const { return outlined; }
        void setOutlineMaterial(const shared_ptr<ShaderMaterial>& mat) { outlineMaterial = mat; }
        shared_ptr<ShaderMaterial>& getOutlineMaterial() { return outlineMaterial; }

    protected:
        shared_ptr<Node> duplicateInstance() override;

    private:
        bool outlined{false};
        shared_ptr<Mesh> mesh;
        shared_ptr<ShaderMaterial> outlineMaterial;
    };

}