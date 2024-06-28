#pragma once

namespace z0 {

    /**
     * Singleton for loading external resources
     */
    class Loader {
    public:
        /**
         * Load a glTF scene
         * @param filepath path of the glTF file, relative to the application path
         * @param forceBackFaceCulling set the z0::CullMode to CULLMODE_BACK even if the material in double sided (default is CULLMODE_DISABLED for double sided materials)
         */
        [[nodiscard]] static shared_ptr<Node> loadModelFromFile(const filesystem::path& filepath, bool forceBackFaceCulling = false);

        /**
         * Creates new instances of nodes described in a JSON file and add them to the parent's tree
         * @param parent Node to add the new nodes to
         * @param filepath path of the glTF file, relative to the application path
         * @param editorMode disable all nodes
         **/       
        static void addSceneFromFile(Node*parent, const filesystem::path& filepath, bool editorMode=false);

        /**
         * Creates new instances of nodes described in a JSON file and add them to the parent's tree
         * @param parent Node to add the new nodes to
         * @param filepath path of the glTF file, relative to the application path
         * @param editorMode disable all nodes
         **/       
        static void addSceneFromFile(shared_ptr<Node>&parent, const filesystem::path& filepath, bool editorMode=false);

        struct SceneNode {
            string id;
            bool isResource;

            string clazz;
            vector<SceneNode> children;
            map<string, string> properties;

            string resource;
            string resourceType;
        };

    private:
        [[nodiscard]] static vector<SceneNode> loadSceneFromJSON(const string& filepath);
        static void addNode(Node* parent, 
                            map<string, shared_ptr<Node>>& nodeTree, 
                            map<string, SceneNode>& sceneTree, 
                            const SceneNode& nodeDesc,
                            bool disableTree);
    };
}
