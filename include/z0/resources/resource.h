#pragma once

#include "z0/object.h"

namespace z0 {

    class Resource: public Object {
    public:
        using id_t = unsigned int;
        explicit Resource(string name);

        id_t getId() const { return id; }
        bool operator==(const Resource& other) const { return id == other.id;}

    protected:
        string name;
        std::string toString() const override { return name.empty() ? Object::toString() : name; };

    private:
        id_t id;
        static id_t currentId;
    };

}