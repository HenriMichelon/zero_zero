module;
#include "z0/modules.h"
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
    class Node: public Object {
    public:
        using id_t = unsigned int;

        /**
         * Creates a node by copying the transforms, process mode, parent and name
         */
        Node(const Node&);
        /**
         * Creates a new node at (0.0, 0.0, 0.0) without parent
         */
        explicit Node(string name = "Node");
        inline virtual ~Node() = default;

        /**
         * Called when a node is ready to initialize, before being added to the scene
         */
        virtual void onReady() {}
        
        /**
         * Called when a node is added to the scene
         */
        virtual void onEnterScene() {};

        /**
         * Called when a node is removed from the scene
         */
        virtual void onExitScene() {};

        /**
         * Called each frame after the physics have been updated and just before drawing the frame
         */
        virtual void onProcess(float alpha) {}

        /**
         * Called just after the physics system have been updated (can be called multiple times if we have free time between frames)
         */
        virtual void onPhysicsProcess(float delta) {}

        /**
         * Called on a keyboard, mouse or gamepas event
         */
        virtual bool onInput(InputEvent& inputEvent) { return false; }

        /**
         * Returns the local space transformation matrix
         */
        [[nodiscard]] inline const mat4& getTransformLocal() const { return localTransform; }

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
        [[nodiscard]] inline vec3 getPosition() const { return localTransform[3]; };

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
        void setRotation(quat quat);

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
        void setProcessMode(ProcessMode mode) { processMode = mode; }

        /**
         * Returns true if the node is processed and receive input callbacks
         */
        [[nodiscard]] bool isProcessed() const;

        /**
         * Returns the node's parent in the scene tree
         */
        [[nodiscard]] inline Node* getParent() const { return parent; }

        /**
         * Adds a child node. 
         * Nodes can have any number of children, but a child can have only one parent.
         */
        bool addChild(const shared_ptr<Node>& child);

        /**
         * Removes a child node. The node, along with its children **can** be deleted depending on their reference counter.
         */
        bool removeChild(const shared_ptr<Node>& child);

        /**
         * Removes all children nodes. The nodes, along with their children **can** be deleted depending on their reference counters.
         */
        void removeAllChildren();

        /**
         * Returns true if the node have this child
         */
        [[nodiscard]] bool haveChild(const shared_ptr<Node>& child, bool recursive) const;

        /*
        * Returns the child node by is name. Not recursive
        */
        [[nodiscard]] shared_ptr<Node> getChild(const string& name) const;

        /*
        * Returns the child node by is absolute path
        */
        [[nodiscard]] shared_ptr<Node> getNode(const string& path) const;

        /**
         * Recursively prints the node tree in the log system
         */
        void printTree(int tab = 0) const;

        [[nodiscard]] string toString() const override { return name; }

        /**
         * Returns the unique ID of this node
         */
        [[nodiscard]] inline id_t getId() const { return id; }

        inline bool operator == (const Node& other) const { return id == other.id;}

        /**
         * Duplicates a node. Warning : not implemented on all nodes types, check documentation for the node type before using it.
         */
        [[nodiscard]] shared_ptr<Node> duplicate();

        /**
         * Finds the first child by is type.
         * Does not works with nodes loaded from a scene file since they are casted to Node.
         */
        template <typename T>
        [[nodiscard]] T* findFirstChild(bool recursive=true) const {
            for(auto& node : children) {
                if (auto* pnode = dynamic_cast<T*>(node.get())) {
                    return pnode;
                } else if (recursive) {
                    return node->findFirstChild<T>(true);
                }
            }
            return nullptr;
        }

        /**
         * Returns the normalized right vector
         */
        [[nodiscard]] vec3 getRightVector() const;

        /**
         * Creates a Tween to tweens a property of the node between an `initial` value 
         * and `final` value in a span of time equal to `duration`, in seconds.
         */
        template<typename T>
        [[nodiscard]] shared_ptr<Tween> createPropertyTween(PropertyTween<T>::Setter set, T initial, T final, float duration) {
            auto tween = make_shared<PropertyTween<T>>(this, set, initial, final, duration);
            tweens.push_back(tween);
            return tween;
        }

        /**
         * Removes the `tween` from the processing list
         */
        void killTween(shared_ptr<Tween>& tween) {
            if (tween != nullptr) {
                tween->_kill();
                tweens.remove(tween);
            }
        }

        /**
         * Sets a property by is name and value. 
         * Currently not all properties in all nodes classes are supported.
         */
        virtual void setProperty(const string&property, const string& value);

        /**
         * Sets the node name (purely informative)
         */
        void setName(const string&nodeName) { name = nodeName; }

        /**
         * Returns the inmutable list of children nodes
         */
        inline const list<shared_ptr<Node>>& getChildren() const { return children; }


    protected:
        string name;
        Node* parent {nullptr};
        list<shared_ptr<Node>> children;
        mat4 localTransform {};
        mat4 worldTransform {};

        virtual shared_ptr<Node> duplicateInstance();
        Application& app();

    private:
        static id_t currentId;
        id_t id;
        ProcessMode processMode{PROCESS_MODE_INHERIT};
        bool inReady{false};
        bool addedToScene{false};
        list<shared_ptr<Tween>> tweens;

    public:
        virtual void _onReady();
        virtual void _onPause() {};
        virtual void _onResume() {};
        inline virtual void _onEnterScene() { onEnterScene(); };
        inline virtual void _onExitScene() { onExitScene(); };
        virtual void _physicsUpdate(float delta);

        void _setParent(Node* p) { parent = p; }
        void _setAddedToScene(bool added) { addedToScene = added; }
        bool _isAddedToScene() { return addedToScene; }
        mat4& _getTransformLocal() { return localTransform; }
        void _setTransform(mat4 transform) { localTransform = transform; }
        virtual void _updateTransform(const mat4& parentMatrix);
        virtual void _updateTransform();
        inline list<shared_ptr<Node>>& _getChildren() { return children; }

    };


    Node::id_t Node::currentId = 0;

    Node::Node(string nodeName):
        name{std::move(nodeName)},
        id{currentId++}{
        replace(name.begin(), name.end(),  '/', '_');
        localTransform = mat4 {1.0};
        _updateTransform(mat4{1.0f});
    }

    Node::Node(const Node& orig): id{currentId++} {
        name = orig.name;
        parent = orig.parent;
        localTransform = orig.localTransform;
        worldTransform = orig.worldTransform;
        processMode = orig.processMode;
    }

    void Node::translate(vec3 localOffset) {
        // current orientation * velocity
        vec3 worldTranslation =  toQuat(mat3(localTransform)) * localOffset;
        setPosition(getPosition() + worldTranslation);
    }

    void Node::setPosition(vec3 pos) {
        localTransform[3] = vec4(pos, 1.0f);
        _updateTransform();
    }

    void Node::setPositionGlobal(vec3 pos) {
        if (parent == nullptr) {
            setPosition(pos);
            return;
        }
        localTransform[3] = inverse(parent->worldTransform) * vec4{pos, 1.0};
        _updateTransform();
    }

    void Node::_updateTransform(const mat4& parentMatrix) {
        worldTransform = parentMatrix * localTransform;
        for (const auto& child : children) {
            child->_updateTransform(worldTransform);
        }
    }

    void Node::_updateTransform() {
        auto parentMatrix = parent == nullptr ? mat4{1.0f} : parent->worldTransform;
        _updateTransform(parentMatrix);
    }

    void Node::rotateX(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_X);
        _updateTransform();
    }

    void Node::rotateY(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Y);
        _updateTransform();
    }

    void Node::rotateZ(float angle) {
        localTransform = rotate(localTransform, angle, AXIS_Z);
        _updateTransform();
    }

    void Node::setRotation(quat quater) {
        vec3 scale, translation, skew;
        vec4 perspective;
        quat orientation;
        // Decompose the original matrix to extract translation, rotation (orientation), and scale
        decompose(localTransform, scale, orientation, translation, skew, perspective);
        // Create a rotation matrix from the new quaternion
        mat4 rotationMatrix = toMat4(quater);
        // Reconstruct the transformation matrix with the new rotation, preserving the original translation and scale
        localTransform = glm::translate(mat4(1.0f), translation)
                         * rotationMatrix
                         * glm::scale(mat4(1.0f), scale);
        _updateTransform();
    }

    vec3 Node::getRotation() const {
        return eulerAngles(toQuat(mat3(localTransform)));
    };

    void Node::setRotationX(float angle) {
        rotateX(angle - getRotationX());
    }

    void Node::setRotationY(float angle) {
        rotateX(angle - getRotationY());
    }

    void Node::setRotationZ(float angle) {
        rotateX(angle - getRotationZ());
    }

    void Node::setScale(float scale) {
        setScale(vec3{scale, scale, scale});
    }

    void Node::setScale(glm::vec3 scale) {
        localTransform = glm::scale(localTransform, scale);
        _updateTransform();
    }

    glm::vec3 Node::getScale() const {
        vec3 scale;
        quat rotation;
        vec3 translation;
        vec3 skew;
        vec4 perspective;
        decompose(localTransform, scale, rotation, translation, skew, perspective);
        return scale;
    }

    void Node::_onReady() {
        inReady = true;
        onReady();
        inReady = false;
    }

    vec3 Node::toGlobal(vec3 local) const {
        return vec3{worldTransform * vec4{local, 1.0f}};
    }

    shared_ptr<Node> Node::getChild(const string& name) const {
        auto it = std::find_if(children.begin(), children.end(), [name](std::shared_ptr<Node> elem) {
            return elem->name == name;
        });
        return it == children.end() ? nullptr : *it;
    }

    shared_ptr<Node> Node::getNode(const string& path) const {
        size_t pos = path.find('/');
        if (pos != std::string::npos) {
            auto child = getChild(path.substr(0, pos));
            if (child != nullptr) {
                return child->getNode(path.substr(pos + 1));
            }
            return nullptr;
        } else {
            return getChild(path);
        }
    }

    /*bool Node::isParent(const shared_ptr<Node>& node) const {
        if (node.get() == parent) { return true; }
        for(const auto& child : children) {
            if (child->isParent(node)) { return true; }
        }
        return false;
    }*/


    bool Node::haveChild(const shared_ptr<z0::Node> &child, bool recursive) const {
        if (recursive) {
            if (haveChild(child, false)) return true;
            for(const auto& node : children) {
                if (node->haveChild(child, true)) return true;
            }
            return false;
        } else {
            return find(children.begin(), children.end(), child) != children.end();
        }
    }

    shared_ptr<Node> Node::duplicate() {
        shared_ptr<Node> dup = duplicateInstance();
        dup->children.clear();
        for(auto&child : children) {
            dup->addChild(child->duplicate());
        }
        dup->id = currentId++;
        dup->name = name;
        return dup;
    }

    shared_ptr<Node> Node::duplicateInstance() {
        return make_shared<Node>(*this);
    }

    void Node::printTree(int tab) const {
        stringstream sstream;
        for (int i = 0; i < (tab*2); i++) {
            sstream << " ";
        }
        sstream << " " << toString();
        log(sstream.str());
        for (auto& child: children) child->printTree(tab+1);
    }

    vec3 Node::getRightVector() const {
        return normalize(mat3{worldTransform} * AXIS_X);
    }

    void Node::_physicsUpdate(float delta) {
        for (auto it = tweens.begin(); it != tweens.end(); ) {
            if ((*it)->update(delta)) {
                it = tweens.erase(it);
            } else {
                ++it;
            }
        }
    }

    void Node::setProperty(const string&property, const string& value) {
        if (property == "position") {
            setPosition(to_vec3(value));
        } else if (property == "rotation") {
            auto rot = to_vec3(value);
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

}