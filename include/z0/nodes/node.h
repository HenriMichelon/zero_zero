#pragma once

#include "z0/object.h"

#include <list>

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;

        Node(const Node&);
        explicit Node(const string& name = "Node");
        virtual ~Node() = default;

        virtual void onReady() {}
        virtual void onProcess(float alpha) {}
        virtual void onPhysicsProcess(float delta) {}

        mat4& getTransform() { return localTransform; }
        virtual void setTransform(mat4 transform) { localTransform = transform; }
        virtual void updateTransform(const mat4& parentMatrix);
        virtual void updateTransform();
        mat4 getTransformGlobal() const { return worldTransform; }

        // parent relative position
        virtual void setPosition(vec3 position);
        vec3 getPosition() const { return localTransform[3]; };

        // world relative position
        vec3 getPositionGlobal() const { return worldTransform[3]; }

        // rotations around own center
        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);

        ProcessMode getProcessMode() const { return processMode; }
        void setProcessMode(ProcessMode mode) { processMode = mode; }
        bool isProcessed() const;

        Node* getParent() const { return parent; }
        bool addChild(const shared_ptr<Node>& child);
        bool removeChild(const shared_ptr<Node>& child);
        list<shared_ptr<Node>>& getChildren() { return children; }
        shared_ptr<Node> getChild(const string& name);
        shared_ptr<Node> getNode(const string& path);
        void printTree(ostream&, int tab = 0);

        id_t getId() const { return id; }
        bool operator == (const Node& other) const { return id == other.id;}
        shared_ptr<Node> duplicate();

    protected:
        string name;
        Node* parent {nullptr};
        list<shared_ptr<Node>> children;
        mat4 localTransform {};
        mat4 worldTransform {};

        string toString() const override { return name; }
        virtual shared_ptr<Node> duplicateInstance();

    private:
        static id_t currentId;
        id_t id;
        ProcessMode processMode{PROCESS_MODE_INHERIT};
        bool inReady{false};

    public:
        virtual void _onReady();
    };

}