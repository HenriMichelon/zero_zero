module;
#include "z0/libraries.h"

export module z0:Resource;

import :Object;

export namespace z0 {

    /**
     * Base class for resources.
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
        static id_t currentId;
    };

}
