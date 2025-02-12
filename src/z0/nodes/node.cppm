/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.nodes.Node;

import z0.Object;
import z0.Constants;
import z0.InputEvent;
import z0.Tween;

 namespace z0 {

    /**
     * Base class for all 3D nodes
     */
    export class Node : public Object {
    public:

        using id_t = unsigned int;

        //! Node type
        enum Type {
            ANIMATION_PLAYER,
            CAMERA,
            CHARACTER,
            COLLISION_AREA,
            COLLISION_OBJECT,
            DIRECTIONAL_LIGHT,
            ENVIRONMENT,
            KINEMATIC_BODY,
            LIGHT,
            MESH_INSTANCE,
            NODE,
            OMNI_LIGHT,
            PHYSICS_BODY,
            RAYCAST,
            RIGID_BODY,
            SKYBOX,
            SPOT_LIGHT,
            STATIC_BODY,
            VIEWPORT
        };

        static constexpr auto TypeNames = array{
            "AnimationPlayer",
            "Camera",
            "Character",
            "CollisionArea",
            "CollisionObject",
            "DirectionalLight",
            "Environment",
            "KinematicBody",
            "Light",
            "MeshInstance",
            "Node",
            "OmniLight",
            "PhysicsBody",
            "RayCast",
            "RigidBody",
            "Skybox",
            "SpotLight",
            "StaticBody",
            "Viewport"
        };

        /**
         * Creates a node by copying the transforms, process mode, type and name
         */
        Node(const Node &orig);

        /**
         * Creates a new node at (0.0, 0.0, 0.0) without parent
         */
        explicit Node(const string &nodeName = "Node", Type type = NODE);

        ~Node() override;

        /**
         * Called when a node is ready to initialize, before being added to the scene
         */
        virtual void onReady() {}

        /**
         * Called when a node is added to the scene
         */
        virtual void onEnterScene() {}

        /**
         * Called when a node is removed from the scene
         */
        virtual void onExitScene() {}

        /**
         * Called each frame after the physics have been updated and just before drawing the frame
         */
        virtual void onProcess(const float alpha) {}

        /**
         * Called just after the physics system have been updated (can be called multiple times if we have free time between frames)
         */
        virtual void onPhysicsProcess(const float delta) {}

        /**
         * Called on a keyboard, mouse or gamepad event
         */
        virtual bool onInput(InputEvent &inputEvent) { return false; }

        /**
         * Returns the local space transformation matrix
         */
        [[nodiscard]] inline const mat4 &getTransformLocal() const { return localTransform; }

        /**
         * Returns the world space transformation matrix
         */
        [[nodiscard]] inline const mat4& getTransformGlobal() const { return worldTransform; }

        /**
         * Transforms a local vector from this node's local space to world space.
         */
        [[nodiscard]] vec3 toGlobal(vec3 local) const;

        /**
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(vec3 position);

        /**
        * Returns the local space position (relative to parent)
        */
        [[nodiscard]] inline vec3 getPosition() const { return localTransform[3]; }

        /**
         * Changes the node's position by the given offset vector in local space.
         */
        void translate(const vec3& localOffset);

        /**
         * Sets the world space position
         */
        virtual void setPositionGlobal(const vec3& position);

        /**
         * Returns the world space position
         */
        [[nodiscard]] inline vec3 getPositionGlobal() const { return worldTransform[3]; }

        /**
         * Rotates the local transformation around the X axis by angle in radians.
         */
        void rotateX(float angle);

        /**
         * Rotates the local transformation around the Y axis by angle in radians.
         */
        void rotateY(float angle);

        /**
         * Rotates the local transformation around the Z axis by angle in radians.
         */
        void rotateZ(float angle);

        /**
         * Rotates the local transformation
         */
        // void rotate(quat quater);

        /**
         * Sets the local transformation
         */
        void setRotation(const quat& quater);

        /**
         * Sets the local transformation
         */
        void setRotation(const vec3& rot);

        /**
         * Sets the X axis rotation of the local transformation by angle in radians.
         */
        void setRotationX(float angle);

        /**
         * Sets the Y axis rotation of the local transformation by angle in radians.
         */
        void setRotationY(float angle);

        /**
         * Sets the Z axis rotation of the local transformation by angle in radians.
         */
        void setRotationZ(float angle);

        /**
         * Returns the rotation of the local transformation, in euler angles in radians
         */
        [[nodiscard]] vec3 getRotation() const;

        /**
         * Returns the rotation of the local transformation
         */
        [[nodiscard]] inline quat getRotationQuaternion() const { return toQuat(mat3(localTransform)); }

        /**
         * Returns the X axis rotation of the local transformation
         */
        [[nodiscard]] inline float getRotationX() const { return getRotation().x; }

        /**
         * Returns the Y axis rotation of the local transformation
         */
        [[nodiscard]] inline float getRotationY() const { return getRotation().y; }

        /**
         * Returns the Z axis rotation of the local transformation
         */
        [[nodiscard]] inline float getRotationZ() const { return getRotation().z; }

        /**
         * Scales part of the local transformation.
         */
        virtual void setScale(const vec3& scale);

        /**
         * Scales part of the local transformation with the same value on each axis
         */
        void setScale(float scale);

        /**
         * Returns the scale part of the local transformation.
         */
        [[nodiscard]] vec3 getScale() const;

        /**
         * Returns the node's processing behavior. To check if the node can process in its current mode, use isProcessed().
         */
        [[nodiscard]] inline ProcessMode getProcessMode() const { return processMode; }

        /**
         * Changes the node's processing behavior.
         */
        void setProcessMode(const ProcessMode mode) { processMode = mode; }

        /**
         * Returns true if the node is processed and receive input callbacks
         */
        [[nodiscard]] bool isProcessed() const;

        /**
         * Returns the node's parent in the scene tree
         */
        [[nodiscard]] inline Node *getParent() const { return parent; }

        /**
         * Adds a child node.<br>
         * Nodes can have any number of children, but a child can have only one parent.<br>
         * The node will be added to the scene at the start of the next frame.
         * @param child the node to add
         * @param async if `true` and the node have children all the nodes will be added in batch mode.
         * Be careful to set the visibility of the nodes to `false`or they will appear slowly in the scene.
         */
        bool addChild(shared_ptr<Node> child, bool async = false);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.<br>
         * Use the iterator version in a for-each loop.<br>
         * The node will be removed from the scene at the start of the next frame.
         * @param child the node to remove
         * @param async if `true` and the node have children all the nodes will be removed in batch mode.
         * Be careful to set the visibility of the nodes to `false` or they will disappear slowly from the scene.
         */
        bool removeChild(const shared_ptr<Node>& child, bool async = false);

        /**
         * Removes all children nodes. The nodes, along with their children **can** be deleted depending on their reference counters.
         * The nodes will be removed from the scene at the start of the next frame.
         * @param async if `true` and the nodes will be removed in batch mode.
         * Be careful to set the visibility of the nodes to `false` or they will disappear slowly from the scene.
         */
        void removeAllChildren(bool async = false);

        /**
         * Returns true if the node have this child
         */
        [[nodiscard]] bool haveChild(const shared_ptr<Node> &child, bool recursive) const;

        /**
        * Returns the child node by is name. Not recursive
        */
        template <typename T = Node>
        [[nodiscard]] shared_ptr<T> getChild(const string &name) const {
            const auto it = std::find_if(children.begin(),
                                         children.end(),
                                         [name](const shared_ptr<Node>& elem) {
                                             return elem->name == name;
                                         });
            return it == children.end() ? nullptr : dynamic_pointer_cast<T>(*it);
        }

        /**
        * Returns the child node by its relative path (does not start with '/')
        */
        template <typename T = Node>
        [[nodiscard]] shared_ptr<T> getChildByPath(const string &path) const {
            const size_t pos = path.find('/');
            if (pos != std::string::npos) {
                const auto child = getChild<Node>(path.substr(0, pos));
                if (child != nullptr) {
                    return child->template getChildByPath<T>(path.substr(pos + 1));
                }
                return nullptr;
            }
            return getChild<T>(path);
        }

        /**
        * Finds the first child by is name.
        */
        template<typename T = Node>
        [[nodiscard]] shared_ptr<T> findFirstChild(const string& name) const {
            for (const auto &node : children) {
                if (node->name == name) {
                    return dynamic_pointer_cast<T>(node);
                }
                if (const auto& found = node->template findFirstChild<T>(name)) {
                    return found;
                }
            }
            return nullptr;
        }

        /**
         * Recursively prints the node tree in the log system
         */
        void printTree(int tab = 0) const;

        [[nodiscard]] inline string toString() const override { return name; }

        /**
         * Returns the unique ID of this node
         */
        [[nodiscard]] inline id_t getId() const { return id; }

        inline bool operator ==(const Node &other) const { return id == other.id; }

        /**
         * Duplicates a node. Warning : not implemented on all nodes types, check documentation for the node type before using it.
         */
        [[nodiscard]] shared_ptr<Node> duplicate();

        /**
         * Finds the first child by is type.
         */
        template <typename T>
        [[nodiscard]] shared_ptr<T> findFirstChild(const bool recursive = true) const {
            for (const auto &node : children) {
                if (const auto& found = dynamic_pointer_cast<T>(node)) {
                    return found;
                }
                if (recursive) {
                    auto result = node->template findFirstChild<T>(true);
                    if (result) {
                        return result;
                    }
                }
            }
            return nullptr;
        }

        /**
         * Finds all children by type
         */
        template <typename T>
        [[nodiscard]] list<shared_ptr<T>> findAllChildren(const bool recursive = true) const {
            list<shared_ptr<T>> result;
            for (const auto &node : children) {
                if (const auto& found = dynamic_pointer_cast<T>(node)) {
                    result.push_back(found);
                }
                if (recursive) {
                    result.append_range(node->template findAllChildren<T>(true));
                }
            }
            return result;
        }

        /**
         * Finds all children by group
         */
        template <typename T = Node>
        [[nodiscard]] list<shared_ptr<T>> findAllChildrenByGroup(const string& groupName, const bool recursive = true) const {
            list<shared_ptr<T>> result;
            for (const auto &node : children) {
                if (isInGroup(groupName)) {
                    result.push_back(dynamic_pointer_cast<T>(node));
                }
                if (recursive) {
                    result.append_range(node->template findAllChildrenByGroup<T>(groupName, true));
                }
            }
            return result;
        }

        /**
         * Returns the normalized right vector
         */
        [[nodiscard]] inline vec3 getRightVector() const {  return normalize(mat3{worldTransform} * AXIS_RIGHT); }

        /**
         * Returns the normalized left vector
         */
        [[nodiscard]] inline vec3 getLeftVector() const {  return normalize(mat3{worldTransform} * AXIS_LEFT); }

        /**
         * Returns the normalized front vector
         */
        [[nodiscard]] inline vec3 getFrontVector() const {  return normalize(mat3{worldTransform} * AXIS_FRONT); }

        /**
         * Returns the normalized back vector
         */
        [[nodiscard]] inline vec3 getBackVector() const {  return normalize(mat3{worldTransform} * AXIS_BACK); }

        /**
         * Returns the normalized up vector
         */
        [[nodiscard]] inline vec3 getUpVector() const {  return normalize(mat3{worldTransform} * AXIS_UP); }

        /**
         * Returns the normalized down vector
         */
        [[nodiscard]] inline vec3 getDownVector() const {  return normalize(mat3{worldTransform} * AXIS_DOWN); }


        /**
         * Creates a Tween to tweens a property of the node between an `initial` value
         * and `final` value in a span of time equal to `duration`, in seconds.
         */
        template <typename T>
        [[nodiscard]] shared_ptr<Tween> createPropertyTween(
                PropertyTween<T>::Setter set,
                T initial,
                T final,
                float duration,
                const TransitionType ttype = TransitionType::LINEAR,
                const Tween::Callback& callback = nullptr) {
            auto tween = make_shared<PropertyTween<T>>(this, set, initial, final, duration, ttype, callback);
            tweens.push_back(tween);
            return tween;
        }

        /**
         * Removes the `tween` from the processing list
         */
        void killTween(const shared_ptr<Tween> &tween);

        /**
         * Sets a property by is name and value.
         * Currently, not all properties in all nodes classes are supported.
         */
        virtual void setProperty(const string &property, const string &value);

        /**
         * Sets the node name (purely informative)
         */
        void setName(const string &nodeName) { name = nodeName; }

        /**
         * Returns the immutable list of children nodes
         */
        inline const list<shared_ptr<Node>> &getChildren() const { return children; }

        /**
         * Returns the node type
         */
        inline Type getType() const { return type; }

        /**
         * Returns the node name
         */
        inline const string &getName() const { return name; }

        /**
         * Returns the node path
         */
        string getPath() const;

        /**
        * Returns a list of group names that the node has been added to.
        */
        inline const list<string>& getGroups() const { return groups; }

        /**
         * Adds the node to the group. Groups can be helpful to organize a subset of nodes, for example "enemies" or "stairs".
         */
        inline void addToGroup(const string &group) { groups.push_back(group); }

        /**
         * Removes the node from the given group. Does nothing if the node is not in the group
         */
        inline void removeFromGroup(const string &group) { groups.remove(group); }

        /**
         * Returns true if this node has been added to the given group
         */
        inline bool isInGroup(const string& group) const { return ranges::find(groups, group) != groups.end(); }

        /**
         * Returns the visibility of the node.
         */
        inline bool isVisible() const { return visible; }

        /**
         * Changes the visibility of the node.<br>
         * The node stays in the scene tree and the data in VRAM.
         */
        virtual void setVisible(bool visible = true);

        /**
         * Returns `true` if this node is currently inside the scene tree
         */
        inline bool isInsideTree() const { return addedToScene; }

        static string sanitizeName(const string &name);

    protected:
        mat4 localTransform{};
        mat4 worldTransform{};

        virtual shared_ptr<Node> duplicateInstance();

    private:
        static id_t             currentId;
        id_t                    id;
        Type                    type;
        string                  name;
        Node *                  parent{nullptr};
        list<shared_ptr<Node>>  children;
        bool                    visible{true};
        ProcessMode             processMode{ProcessMode::INHERIT};
        bool                    isReady{false};
        bool                    addedToScene{false};
        list<shared_ptr<Tween>> tweens;
        list<string>            groups;

    public:
        virtual void _onReady();

        inline virtual void _onPause() { }

        inline virtual void _onResume() { }

        inline virtual void _onEnterScene() { onEnterScene(); }

        inline virtual void _onExitScene() { onExitScene(); }

        virtual void _physicsUpdate(float delta);

        virtual void _update(float alpha) {}

        inline void _setParent(Node *p) { parent = p; }

        inline void _setAddedToScene(const bool added) { addedToScene = added; }

        inline bool _isAddedToScene() const { return addedToScene; }

        inline mat4 &_getTransformLocal() { return localTransform; }

        inline void _setTransform(const mat4 &transform) { localTransform = transform; }

        virtual void _updateTransform(const mat4 &parentMatrix);

        virtual void _updateTransform();

        inline list<shared_ptr<Node>> &_getChildren() { return children; }

    };

}
