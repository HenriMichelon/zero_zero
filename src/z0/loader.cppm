/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Loader;

import z0.Node;

export namespace z0 {
    /**
     * Singleton for loading external resources
     */
    class Loader {
    public:
        /**
         * Load a glTF or ZScene scene
         * @param filepath path of the glTF/ZScene file, relative to the application path
         */
        [[nodiscard]] static shared_ptr<Node> load(const string& filepath);

        static void _cleanup();

        // Node description inside a JSON file
        struct SceneNode {
            string id{};
            bool   isResource{false};
            bool   isCustom{false};
            bool   isIncluded{false};

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
        static inline map<string, shared_ptr<Node>> resources;
        static mutex resourcesMutex;

        static shared_ptr<Node> loadScene(const string &filepath);

        [[nodiscard]] static vector<SceneNode> loadSceneDescriptionFromJSON(const string &filepath);

        static void addNode(Node *                         parent,
                            map<string, shared_ptr<Node>> &nodeTree,
                            map<string, SceneNode> &       sceneTree,
                            const SceneNode &              nodeDesc);
    };

}
