#pragma once

namespace z0 {

    /**
     * Base class for anything
     */
    class Object {
    public:
        friend ostream& operator<<(ostream& os, const Object& obj) {
            os << obj.toString();
            return os;
        }

        /**
         * Convert the objet to a readable text
         */
        virtual string toString() const { return "??"; };
    };

}