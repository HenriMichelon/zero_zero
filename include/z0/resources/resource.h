#pragma once

namespace z0 {

    class Resource: public Object {
    public:
        using id_t = unsigned int;
        explicit Resource(string  name);

        id_t getId() const { return id; }
        const string& getName() const { return name; }
        bool operator==(const Resource& other) const { return id == other.id;}

    protected:
        string name;
        string toString() const override { return name; }

    private:
        id_t id;
        static id_t currentId;
    };

}