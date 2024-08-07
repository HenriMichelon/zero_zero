#pragma once

namespace z0 {

    /**
     * Base class for resources.
     * All resources have a reference counter that *can* be used by the renderers to manage the resources of the scene.
     */
    class Resource: public Object {
    public:
        using id_t = unsigned int;
        explicit Resource(string  name);

        /**
         * Returns the unique id of the resource
         */
        [[nodiscard]] id_t getId() const { return id; }

        /**
         * Return the name (only informative, no real use)
         */
        [[nodiscard]] const string& getName() const { return name; }

        bool operator==(const Resource& other) const { return id == other.id;}

    protected:
        string name;
        string toString() const override { return name; }

    private:
        id_t id;
        uint32_t refCount{0};
        static id_t currentId;

    public:
        void _incrementReferenceCounter();
        [[nodiscard]] bool _decrementReferenceCounter();
    };

}