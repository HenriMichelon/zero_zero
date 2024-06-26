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
        static shared_ptr<Node> loadModelFromFile(const filesystem::path& filepath, bool forceBackFaceCulling = false);

        /**
         * Creates new instances of nodes described in a JSON file and add them to the parent's tree
         * @param parent Node to add the new nodes to
         * @param filepath path of the glTF file, relative to the application path
         **/       
        static void addSceneFromFile(shared_ptr<Node>&parent, const filesystem::path& filepath);

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
        static vector<SceneNode> loadSceneFromJSON(const string& filepath);
        static void addNode(shared_ptr<Node>& parent, 
                            map<string, shared_ptr<Node>>& nodeTree, 
                            map<string, SceneNode>& sceneTree, 
                            const SceneNode& nodeDesc);
    };
}
