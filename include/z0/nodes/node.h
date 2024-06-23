#pragma once

namespace z0 {

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
        virtual ~Node() = default;

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
        inline const mat4& getTransformLocal() const { return localTransform; }

        /**
         * Returns the world space transformation matrix
         */
        inline mat4 getTransformGlobal() const { return worldTransform; }

        /**
         * Transforms a local vector from this node's local space to world space.
         */
        vec3 toGlobal(vec3 local) const;

        /*
        * Sets the local space position (relative to parent)
        */
        virtual void setPosition(vec3 position);

        /*
        * Returns the local space position (relative to parent)
        */
        inline vec3 getPosition() const { return localTransform[3]; };

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
        inline vec3 getPositionGlobal() const { return worldTransform[3]; }

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
        vec3 getRotation() const;

        /**
         * Returns the X axis rotation of the local transformation
         */
        float getRotationX() const { return getRotation().x; }

        /**
         * Returns the Y axis rotation of the local transformation
         */
        float getRotationY() const { return getRotation().y; }

        /**
         * Returns the Z axis rotation of the local transformation
         */
        float getRotationZ() const { return getRotation().z; }

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
        vec3 getScale() const;

        /**
         * Returns the node's processing behavior. To check if the node can process in its current mode, use isProcessed().
         */
        inline ProcessMode getProcessMode() const { return processMode; }

        /**
         * Changes the node's processing behavior.
         */
        void setProcessMode(ProcessMode mode) { processMode = mode; }

        /**
         * Returns true if the node is processed and receive input callbacks
         */
        bool isProcessed() const;

        /**
         * Returns the node's parent in the scene tree
         */
        inline Node* getParent() const { return parent; }

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
        bool haveChild(const shared_ptr<Node>& child, bool recursive) const;

        /*
        * Returns the child node by is name. Not recursive
        */
        shared_ptr<Node> getChild(const string& name) const;

        /*
        * Returns the child node by is absolute path
        */
        shared_ptr<Node> getNode(const string& path) const;

        /**
         * Recursively prints the node tree in the log system
         */
        void printTree(int tab = 0) const;

        string toString() const override { return name; }

        /**
         * Returns the unique ID of this node
         */
        inline id_t getId() const { return id; }

        inline bool operator == (const Node& other) const { return id == other.id;}

        /**
         * Duplicates a node. Warning : not implemented on all nodes types, check documentation for the node type before using it.
         */
        shared_ptr<Node> duplicate();

        /**
         * Finds the first child by is type
         */
        template <typename T>
        T* findFirstChild(bool recursive=true) const {
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
        vec3 getRightVector() const;

        /**
         * Creates a Tween to tweens a property of the node between an `initial` value 
         * and `final` value in a span of time equal to `duration`, in seconds.
         */
        template<typename T>
        shared_ptr<Tween> createPropertyTween(PropertyTween<T>::Setter set, T initial, T final, float duration) {
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

        void _setAddedToScene(bool added) { addedToScene = added; }
        bool _isAddedToScene() { return addedToScene; }
        mat4& _getTransformLocal() { return localTransform; }
        void _setTransform(mat4 transform) { localTransform = transform; }
        virtual void _updateTransform(const mat4& parentMatrix);
        virtual void _updateTransform();
        inline list<shared_ptr<Node>>& _getChildren() { return children; }

    };

}