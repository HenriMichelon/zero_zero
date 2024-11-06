/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.Resource;

import z0.Tools;
import z0.Object;

export namespace z0 {

    /**
     * Base class for resources.
     */
    class Resource : public Object {
    public:
        using id_t = int64_t;
        static constexpr id_t INVALID_ID = -1;

        explicit Resource(string name);

        // ~Resource() override { log("~Resource", name, to_string(getId())); }

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

        /**
         * Duplicates a resource. Warning : not implemented on all resources types, check documentation for the resource type before using it.
         */
        [[nodiscard]] virtual shared_ptr<Resource> duplicate() const { die("Resource::duplicate() not implemented for this resource type", name); return nullptr; };

    protected:
        string name;

    private:
        id_t        id;
        static id_t currentId;
    };

}
