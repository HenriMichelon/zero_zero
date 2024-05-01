#pragma once

#include "z0/object.h"

#include <list>

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;

        Node(const Node&);
        explicit Node(string name = "Node");
        virtual ~Node() = default;

        virtual void onReady() {}

        glm::mat4& getTransform() { return localTransform; }
        virtual void setTransform(glm::mat4 transform) { localTransform = transform; }
        virtual void updateTransform(const mat4& parentMatrix);

        ProcessMode getProcessMode() const { return processMode; }
        void setProcessMode(ProcessMode mode) { processMode = mode; }
        bool isProcessed() const;

        Node* getParent() const { return parent; }
        void addChild(const shared_ptr<Node>& child);

        id_t getId() const { return id; }
        bool operator == (const Node& other) const { return id == other.id;}

    protected:
        string name;
        Node* parent {nullptr};
        list<shared_ptr<Node>> children;
        mat4 localTransform {};
        mat4 worldTransform {};

        string toString() const override { return name; }

    private:
        static id_t currentId;
        const id_t id;
        ProcessMode processMode{PROCESS_MODE_INHERIT};

    };

}