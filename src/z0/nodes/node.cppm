module;
#include <cstdlib>
#include "z0/libraries.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

export module Z0:Node;

import :Object;
import :Constants;
import :InputEvent;
import :Tween;
import :Tools;

export namespace z0 {

    class Application;

    /**
     * Base class for all 3D nodes
     */
    class Node : public Object {
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

        /**
         * Creates a node by copying the transforms, process mode, parent and name
         */
        Node(const Node &orig):
            id{currentId++} {
            name           = orig.name;
            parent         = orig.parent;
            localTransform = orig.localTransform;
            worldTransform = orig.worldTransform;
            processMode    = orig.processMode;
            type           = orig.type;
        }

        /**
         * Creates a new node at (0.0, 0.0, 0.0) without parent
         */
        explicit Node(const string &nodeName = "Node", Type type = NODE):
            type{type},
            name{std::move(nodeName)},
            id{currentId++} {
            replace(name.begin(), name.end(), '/', '_');
            localTransform = mat4{1.0};
            _updateTransform(mat4{1.0f});
        }

        ~Node() override = default;

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
        [[nodiscard]] vec3 toGlobal(const vec3 local) const {
            return vec3{worldTransform * vec4{local, 1.0f}};
        }

        /*
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(const vec3 position) {
            localTransform[3] = vec4{position, 1.0f};
            _updateTransform();
        }

        /*
        * Returns the local space position (relative to parent)
        */
        [[nodiscard]] inline vec3 getPosition() const { return localTransform[3]; }

        /**
         * Changes the node's position by the given offset vector in local space.
         */
        void translate(const vec3 localOffset) {
            // current orientation * velocity
            const vec3 worldTranslation = toQuat(mat3(localTransform)) * localOffset;
            setPosition(getPosition() + worldTranslation);
        }

        /**
         * Sets the world space position
         */
        virtual void setPositionGlobal(const vec3 position) {
            if (parent == nullptr) {
                setPosition(position);
                return;
            }
            localTransform[3] = inverse(parent->worldTransform) * vec4{position, 1.0};
            _updateTransform();
        }

        /**
         * Returns the world space position
         */
        [[nodiscard]] inline vec3 getPositionGlobal() const { return worldTransform[3]; }

        /**
         * Rotates the local transformation around the X axis by angle in radians.
         */
        void rotateX(const float angle) {
            localTransform = rotate(localTransform, angle, AXIS_X);
            _updateTransform();
        }

        /**
         * Rotates the local transformation around the Y axis by angle in radians.
         */
        void rotateY(const float angle) {
            localTransform = rotate(localTransform, angle, AXIS_Y);
            _updateTransform();
        }

        /**
         * Rotates the local transformation around the Z axis by angle in radians.
         */
        void rotateZ(const float angle) {
            localTransform = rotate(localTransform, angle, AXIS_Z);
            _updateTransform();
        }

        /**
         * Sets the local transformation
         */
        void setRotation(const quat quater) {
            vec3 scale, translation, skew;
            vec4 perspective;
            quat orientation;
            // Decompose the original matrix to extract translation, rotation (orientation), and scale
            decompose(localTransform, scale, orientation, translation, skew, perspective);
            // Create a rotation matrix from the new quaternion
            mat4 rotationMatrix = toMat4(quater);
            // Reconstruct the transformation matrix with the new rotation, preserving the original translation and scale
            localTransform = glm::translate(mat4{1.0f}, translation)
                    * rotationMatrix
                    * glm::scale(mat4{1.0f}, scale);
            _updateTransform();
        }

        /**
         * Sets the X axis rotation of the local transformation by angle in radians.
         */
        void setRotationX(const float angle) {
            rotateX(angle - getRotationX());
        }

        /**
         * Sets the Y axis rotation of the local transformation by angle in radians.
         */
        void setRotationY(const float angle) {
            rotateX(angle - getRotationY());
        }

        /**
         * Sets the Z axis rotation of the local transformation by angle in radians.
         */
        void setRotationZ(const float angle) {
            rotateX(angle - getRotationZ());
        }

        /**
         * Returns the rotation of the local transformation
         */
        [[nodiscard]] vec3 getRotation() const {
            return eulerAngles(toQuat(mat3(localTransform)));
        };

        /**
         * Returns the X axis rotation of the local transformation
         */
        [[nodiscard]] float getRotationX() const { return getRotation().x; }

        /**
         * Returns the Y axis rotation of the local transformation
         */
        [[nodiscard]] float getRotationY() const { return getRotation().y; }

        /**
         * Returns the Z axis rotation of the local transformation
         */
        [[nodiscard]] float getRotationZ() const { return getRotation().z; }

        /**
         * Scales part of the local transformation.
         */
        virtual void setScale(const vec3 scale) {
            localTransform = glm::scale(localTransform, scale);
            _updateTransform();
        }

        /**
         * Scales part of the local transformation with the same value on each axis
         */
        void setScale(const float scale) {
            setScale(vec3{scale, scale, scale});
        }

        /**
         * Returns the scale part of the local transformation.
         */
        [[nodiscard]] vec3 getScale() const {
            vec3 scale;
            quat rotation;
            vec3 translation;
            vec3 skew;
            vec4 perspective;
            decompose(localTransform, scale, rotation, translation, skew, perspective);
            return scale;
        }

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
         * Adds a child node.
         * Nodes can have any number of children, but a child can have only one parent.
         */
        bool addChild(const shared_ptr<Node> &child);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.
         */
        bool removeChild(const shared_ptr<Node> &child);

        /**
         * Removes all children nodes. The nodes, along with their children **can** be deleted depending on their reference counters.
         */
        void removeAllChildren();

        /**
         * Returns true if the node have this child
         */
        [[nodiscard]] bool haveChild(const shared_ptr<Node> &child, const bool recursive) const {
            if (recursive) {
                if (haveChild(child, false))
                    return true;
                for (const auto &node : children) {
                    if (node->haveChild(child, true))
                        return true;
                }
                return false;
            }
            return find(children.begin(), children.end(), child) != children.end();
        }

        /*
        * Returns the child node by is name. Not recursive
        */
        [[nodiscard]] shared_ptr<Node> getChild(const string &name) const {
            auto it = std::find_if(children.begin(),
                                   children.end(),
                                   [name](std::shared_ptr<Node> elem) {
                                       return elem->name == name;
                                   });
            return it == children.end() ? nullptr : *it;
        }

        /*
        * Returns the child node by is absolute path
        */
        [[nodiscard]] shared_ptr<Node> getNode(const string &path) const {
            size_t pos = path.find('/');
            if (pos != std::string::npos) {
                auto child = getChild(path.substr(0, pos));
                if (child != nullptr) {
                    return child->getNode(path.substr(pos + 1));
                }
                return nullptr;
            }
            return getChild(path);
        }

        /**
         * Recursively prints the node tree in the log system
         */
        void printTree(const int tab = 0) const {
            stringstream sstream;
            for (int i = 0; i < (tab * 2); i++) {
                sstream << " ";
            }
            sstream << " " << toString();
            log(sstream.str());
            for (auto &child : children)
                child->printTree(tab + 1);
        }

        [[nodiscard]] string toString() const override { return name; }

        /**
         * Returns the unique ID of this node
         */
        [[nodiscard]] inline id_t getId() const { return id; }

        inline bool operator ==(const Node &other) const { return id == other.id; }

        /**
         * Duplicates a node. Warning : not implemented on all nodes types, check documentation for the node type before using it.
         */
        [[nodiscard]] shared_ptr<Node> duplicate() {
            shared_ptr<Node> dup = duplicateInstance();
            dup->children.clear();
            for (const auto &child : children) {
                dup->addChild(child->duplicate());
            }
            dup->id   = currentId++;
            dup->name = name;
            return dup;
        }

        /**
         * Finds the first child by is type.
         * Does not work with nodes loaded from a scene file since they are cast to Node.
         */
        template <typename T>
        [[nodiscard]] T *findFirstChild(bool recursive = true) const {
            for (auto &node : children) {
                if (auto *pnode = dynamic_cast<T *>(node.get())) {
                    return pnode;
                }
                if (recursive) {
                    return node->findFirstChild<T>(true);
                }
            }
            return nullptr;
        }

        /**
         * Returns the normalized right vector
         */
        [[nodiscard]] vec3 getRightVector() const {
            return normalize(mat3{worldTransform} * AXIS_X);
        }

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
        void killTween(const shared_ptr<Tween> &tween) {
            if (tween != nullptr) {
                tween->_kill();
                tweens.remove(tween);
            }
        }

        /**
         * Sets a property by is name and value.
         * Currently, not all properties in all nodes classes are supported.
         */
        virtual void setProperty(const string &property, const string &value) {
            if (property == "position") {
                setPosition(to_vec3(value));
            } else if (property == "rotation") {
                const auto rot = to_vec3(value);
                setRotation(vec3{radians(rot.x), radians(rot.y), radians(rot.z)});
            } else if (property == "process_mode") {
                auto v = to_lower(value);
                if (v == "inherit") {
                    setProcessMode(PROCESS_MODE_INHERIT);
                } else if (v == "pausable") {
                    setProcessMode(PROCESS_MODE_PAUSABLE);
                } else if (v == "when_paused") {
                    setProcessMode(PROCESS_MODE_WHEN_PAUSED);
                } else if (v == "always") {
                    setProcessMode(PROCESS_MODE_ALWAYS);
                } else if (v == "disabled") {
                    setProcessMode(PROCESS_MODE_DISABLED);
                }
            }
        }

        /**
         * Sets the node name (purely informative)
         */
        void setName(const string &nodeName) { name = nodeName; }

        /**
         * Returns the immutable list of children nodes
         */
        inline const list<shared_ptr<Node>> &getChildren() const { return children; }

        /**
         * Return the node type
         */
        Type getType() const { return type; }

    protected:
        Type                   type;
        string                 name;
        Node *                 parent{nullptr};
        list<shared_ptr<Node>> children;
        mat4                   localTransform{};
        mat4                   worldTransform{};

        virtual shared_ptr<Node> duplicateInstance() {
            return make_shared<Node>(*this);
        }

        Application &app() const;

    private:
        static id_t             currentId;
        id_t                    id;
        ProcessMode             processMode{PROCESS_MODE_INHERIT};
        bool                    inReady{false};
        bool                    addedToScene{false};
        list<shared_ptr<Tween>> tweens;

    public:
        virtual void _onReady() {
            inReady = true;
            onReady();
            inReady = false;
        }

        virtual void _onPause() {
        }

        virtual void _onResume() {
        }

        inline virtual void _onEnterScene() { onEnterScene(); }

        inline virtual void _onExitScene() { onExitScene(); }

        virtual void _physicsUpdate(const float delta) {
            for (auto it = tweens.begin(); it != tweens.end();) {
                if ((*it)->update(delta)) {
                    it = tweens.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void _setParent(Node *p) { parent = p; }

        void _setAddedToScene(const bool added) { addedToScene = added; }

        bool _isAddedToScene() const { return addedToScene; }

        mat4 &_getTransformLocal() { return localTransform; }

        void _setTransform(const mat4 &transform) { localTransform = transform; }

        virtual void _updateTransform(const mat4 &parentMatrix) {
            worldTransform = parentMatrix * localTransform;
            for (const auto &child : children) {
                child->_updateTransform(worldTransform);
            }
        }

        virtual void _updateTransform() {
            const auto parentMatrix = parent == nullptr ? mat4{1.0f} : parent->worldTransform;
            _updateTransform(parentMatrix);
        }

        inline list<shared_ptr<Node>> &_getChildren() { return children; }

    };

    Node::id_t Node::currentId = 0;

}
