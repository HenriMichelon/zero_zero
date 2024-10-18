module;
#include "z0/libraries.h"

export module z0:Resource;

import :Object;

export namespace z0 {

    /**
     * Base class for resources.
     * All resources have a reference counter that *can* be used by the renderers to manage the resources of the scene.
     */
    class Resource : public Object {
    public:
        using id_t = int64_t;
        static constexpr id_t INVALID_ID = -1;

        explicit Resource(string name);

        /**
         * Returns the unique id of the resource
         */
        [[nodiscard]] inline id_t getId() const { return id; }

        /**
         * Return the name (only informative, no real use)
         */
        [[nodiscard]] inline const string &getName() const { return name; }

        inline bool operator==(const Resource &other) const { return id == other.id; }

        inline bool operator<(const Resource &other) const { return id < other.id; }

        inline bool operator>(const Resource &other) const { return id > other.id; }

        [[nodiscard]] inline string toString() const override { return name; }

    protected:
        string name;

    private:
        id_t        id;
        uint32_t    refCount{0};
        static id_t currentId;

    public:
        inline void _incrementReferenceCounter() { refCount += 1; }

        [[nodiscard]] inline bool _decrementReferenceCounter() {
            refCount -= 1;
            return refCount == 0;
        }
    };

}
