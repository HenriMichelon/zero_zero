module;
#include "z0/modules.h"

export module Z0:Resource;

import :Object;

export namespace z0 {

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


    Resource::id_t Resource::currentId = 0;

    Resource::Resource(string  resName):
        name{std::move(resName)},
        id{currentId++} {
        replace(name.begin(), name.end(),  '/', '_');
        replace(name.begin(), name.end(),  '\\', '_');
        replace(name.begin(), name.end(),  ':', '_');
    }

    void Resource::_incrementReferenceCounter() {
        refCount += 1;
    }

    bool Resource::_decrementReferenceCounter() {
        refCount -= 1;
        return refCount == 0;
    }

}