/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Loader;

import z0.nodes.Node;

export namespace z0 {
    /**
     * Singleton for loading external resources
     */
    class Loader {
    public:
        /**
         * Load a JSON, glTF or ZRes file
         * @param filepath path of the JSON/glTF/ZRes file, relative to the application path
         * @param usecache put loaded resources in the global resources cache
         */
        template<typename T = Node>
        [[nodiscard]] static shared_ptr<T> load(const string& filepath, bool usecache = false) {
            if (usecache) {
                auto lock = lock_guard(resourcesMutex);
                if (resources.contains(filepath)) {
                    // log("re-using resources", filepath);
                    return dynamic_pointer_cast<T>(resources[filepath]);
                }
            }
            const auto rootNode = make_shared<T>(filepath);
            load(rootNode, filepath, usecache);
            return rootNode;
        }

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

        [[nodiscard]] static void load(const shared_ptr<Node>&rootNode, const string& filepath, bool usecache);

        static void loadScene(const shared_ptr<Node>&rootNode, const string &filepath);

        [[nodiscard]] static vector<SceneNode> loadSceneDescriptionFromJSON(const string &filepath);

        static void addNode(Node *                         parent,
                            map<string, shared_ptr<Node>> &nodeTree,
                            map<string, SceneNode> &       sceneTree,
                            const SceneNode &              nodeDesc);
    };

}
