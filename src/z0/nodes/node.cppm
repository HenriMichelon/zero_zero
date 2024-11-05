/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Node;

import z0.Object;
import z0.Constants;
import z0.InputEvent;
import z0.Tween;

 namespace z0 {

     class Application;

    /**
     * Base class for all 3D nodes
     */
    export class Node : public Object {
    public:

        using id_t = unsigned int;

        //! Node type
        enum Type {
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

        ~Node() override {};

        /**
         * Called when a node is ready to initialize, before being added to the scene
         */
        virtual void onReady() {
        }

        /**
         * Called when a node is added to the scene
         */
        virtual void onEnterScene() {
        }

        /**
         * Called when a node is removed from the scene
         */
        virtual void onExitScene() {
        }

        /**
         * Called each frame after the physics have been updated and just before drawing the frame
         */
        virtual void onProcess(const float alpha) {
        }

        /**
         * Called just after the physics system have been updated (can be called multiple times if we have free time between frames)
         */
        virtual void onPhysicsProcess(const float delta) {
        }

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
        [[nodiscard]] inline mat4 getTransformGlobal() const { return worldTransform; }

        /**
         * Transforms a local vector from this node's local space to world space.
         */
        [[nodiscard]] vec3 toGlobal(vec3 local) const;

        /*
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(vec3 position);

        /*
        * Returns the local space position (relative to parent)
        */
        [[nodiscard]] inline vec3 getPosition() const { return localTransform[3]; }

        /**
         * Changes the node's position by the given offset vector in local space.
         */
        void translate(vec3 localOffset);

        /**
         * Sets the world space position
         */
        virtual void setPositionGlobal(vec3 position);

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
         * Sets the local transformation
         */
        void setRotation(quat quater);

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
         * Returns the rotation of the local transformation
         */
        [[nodiscard]] vec3 getRotation() const;

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
        virtual void setScale(vec3 scale);

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
         */
        bool addChild(shared_ptr<Node> child);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.<br>
         * Use the iterator version in a for-each loop.<br>
         * The node will be removed from the scene at the start of the next frame.
         */
        bool removeChild(const shared_ptr<Node> &child);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.
         */
        list<shared_ptr<Node>>::const_iterator removeChild(const list<shared_ptr<Node>>::const_iterator &it);

        /**
         * Removes all children nodes. The nodes, along with their children **can** be deleted depending on their reference counters.
         */
        void removeAllChildren();

        /**
         * Returns true if the node have this child
         */
        [[nodiscard]] bool haveChild(const shared_ptr<Node> &child, bool recursive) const;

        /*
        * Returns the child node by is name. Not recursive
        */
        [[nodiscard]] shared_ptr<Node> getChild(const string &name) const;

        /*
        * Returns the child node by is absolute path
        */
        [[nodiscard]] shared_ptr<Node> getNode(const string &path) const;

        /**
        * Finds the first child by is name.
        */
        [[nodiscard]] shared_ptr<Node> findFirstChild(const string& name) const;

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
         * Does not work with nodes loaded from a scene file since they are cast to Node.
         */
        template <typename T>
        [[nodiscard]] shared_ptr<T> findFirstChild(const bool recursive = true) const {
            for (const auto &node : children) {
                if (const auto& found = dynamic_pointer_cast<T>(node)) {
                    return found;
                }
                if (recursive) {
                    return node->findFirstChild<T>(true);
                }
            }
            return {nullptr};
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
        [[nodiscard]] shared_ptr<Tween> createPropertyTween(PropertyTween<T>::Setter set, T initial, T final,
                                                            float                    duration) {
            auto tween = make_shared<PropertyTween<T>>(this, set, initial, final, duration);
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

    protected:
        Type                   type;
        string                 name;
        Node *                 parent{nullptr};
        list<shared_ptr<Node>> children;
        mat4                   localTransform{};
        mat4                   worldTransform{};

        virtual shared_ptr<Node> duplicateInstance();

        Application &app() const;

    private:
        static id_t             currentId;
        id_t                    id;
        ProcessMode             processMode{PROCESS_MODE_INHERIT};
        bool                    inReady{false};
        bool                    addedToScene{false};
        list<shared_ptr<Tween>> tweens;

    public:
        virtual void _onReady();

        inline virtual void _onPause() {
        }

        inline virtual void _onResume() {
        }

        inline virtual void _onEnterScene() { onEnterScene(); }

        inline virtual void _onExitScene() { onExitScene(); }

        virtual void _physicsUpdate(float delta);

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
