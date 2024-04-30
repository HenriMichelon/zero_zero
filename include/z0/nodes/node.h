#pragma once

#include "z0/object.h"

namespace z0 {

    class Node: public Object {
    public:
        using id_t = unsigned int;

        explicit Node(string name = "Node");
        Node(const Node&);
        virtual ~Node() = default;

        virtual void onReady() {}

        id_t getId() const { return id; }

    protected:
        string name;

        string toString() const override { return name; }

    private:
        id_t id;
        static id_t currentId;

    };

}