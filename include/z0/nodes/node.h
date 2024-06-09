#pragma once

namespace z0 {

    class Application;

    class Node: public Object {
    public:
        using id_t = unsigned int;

        Node(const Node&);
        explicit Node(string name = "Node");
        virtual ~Node() = default;

        virtual void onReady() {}
        virtual void onProcess(float alpha) {}
        virtual void onPhysicsProcess(float delta) {}
        virtual bool onInput(InputEvent& inputEvent) { return false; }

        mat4& getTransform() { return localTransform; }
        virtual void setTransform(mat4 transform) { localTransform = transform; }
        virtual void updateTransform(const mat4& parentMatrix);
        virtual void updateTransform();
        mat4 getTransformGlobal() const { return worldTransform; }
        vec3 toGlobal(vec3 local) const;

        // parent relative position
        virtual void setPosition(vec3 position);
        vec3 getPosition() const { return localTransform[3]; };
        void translate(vec3 localOffset);

        // world relative position
        virtual void setPositionGlobal(vec3 position);
        vec3 getPositionGlobal() const { return worldTransform[3]; }

        // rotations around own center
        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);
        void setRotation(quat quat);
        void setRotationX(float angle);
        void setRotationY(float angle);
        void setRotationZ(float angle);
        vec3 getRotation() const;
        float getRotationX() const { return getRotation().x; }
        float getRotationY() const { return getRotation().y; }
        float getRotationZ() const { return getRotation().z; }

        virtual void setScale(vec3 scale);
        void setScale(float scale);
        vec3 getScale() const;

        ProcessMode getProcessMode() const { return processMode; }
        void setProcessMode(ProcessMode mode) { processMode = mode; }
        bool isProcessed() const;

        Node* getParent() const { return parent; }
        bool addChild(const shared_ptr<Node>& child);
        bool removeChild(const shared_ptr<Node>& child);
        void removeAllChildren();
        bool haveChild(const shared_ptr<Node>& child, bool recursive) const;
        list<shared_ptr<Node>>& getChildren() { return children; }
        shared_ptr<Node> getChild(const string& name) const;
        shared_ptr<Node> getNode(const string& path) const;
        void printTree(int tab = 0) const;
        string toString() const override { return name; }
        //bool isParent(const shared_ptr<Node>&) const;

        id_t getId() const { return id; }
        bool operator == (const Node& other) const { return id == other.id;}
        shared_ptr<Node> duplicate();

        template <typename T>
        T* findFirstChild(bool recursive) const {
            for(auto& node : children) {
                if (auto* pnode = dynamic_cast<T*>(node.get())) {
                    return pnode;
                } else if (recursive) {
                    return node->findFirstChild<T>(true);
                }
            }
            return nullptr;
        }

    protected:
        string name;
        Node* parent {nullptr};
        list<shared_ptr<Node>> children;
        mat4 localTransform {};
        mat4 worldTransform {};
        bool needPhysics{false};

        virtual shared_ptr<Node> duplicateInstance();
        Application& app();

    private:
        static id_t currentId;
        id_t id;
        ProcessMode processMode{PROCESS_MODE_INHERIT};
        bool inReady{false};
        bool addedToScene{false};

    public:
        virtual void _onReady();
        virtual void _onEnterScene() {};
        virtual void _onExitScene() {};
        virtual void _physicsUpdate() {};

        bool _needPhysics() const { return needPhysics; }
        void _setAddedToScene(bool added) { addedToScene = added; }
        bool _isAddedToScene() { return addedToScene; }
    };

}