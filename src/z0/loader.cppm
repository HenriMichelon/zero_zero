module;
#include "z0/libraries.h"

export module z0:Loader;

import :Node;
import :MeshInstance;

export namespace z0 {
    /**
     * Singleton for loading external resources
     */
    class Loader {
    public:
        /**
         * Load a glTF scene
         * @param filepath path of the glTF file, relative to the application path
         * @param forceBackFaceCulling set the z0::CullMode to CULLMODE_BACK even if the material is double-sided (default is CULLMODE_DISABLED for double sided materials)
         * @param loadTextures do not load image
         */
        [[nodiscard]] static shared_ptr<Node> loadModelFromFile(const string& filepath,
                                                                bool forceBackFaceCulling = false,
                                                                bool loadTextures         = true);

        /**
         * Creates new instances of nodes described in a JSON file and add them to the parent's tree
         * @param parent Node to add the new nodes to
         * @param filepath path of the glTF file, relative to the application path
         * @param editorMode disable all nodes
         **/
        static void addSceneFromFile(Node *parent, const filesystem::path &filepath, bool loadTextures = true);

        /**
         * Creates new instances of nodes described in a JSON file and add them to the parent's tree
         * @param parent Node to add the new nodes to
         * @param filepath path of the glTF file, relative to the application path
         * @param editorMode disable all nodes
         **/
        static void addSceneFromFile(shared_ptr<Node> &parent, const filesystem::path &filepath,
                                     bool              loadTextures = true);

        // Node description inside a JSON file
        struct SceneNode {
            string id{};
            bool   isResource{false};
            bool   isCustom{false};

            string                            clazz{};
            shared_ptr<SceneNode>             child{nullptr};
            vector<SceneNode>                 children{};
            vector<std::pair<string, string>> properties{};
            // using a vector of pairs to preserve JSON declaration order

            string resource{};
            string resourcePath{};
            string resourceType{};
            bool   needDuplicate{false};
        };

    private:
        [[nodiscard]] static vector<SceneNode> loadSceneFromJSON(const filesystem::path &filepath);

        static void addNode(Node *                         parent,
                            map<string, shared_ptr<Node>> &nodeTree,
                            map<string, SceneNode> &       sceneTree,
                            const SceneNode &              nodeDesc,
                            bool                           loadTextures);
    };

}
