#pragma once

#include "z0/tools.h"

namespace z0 {

    class Object {
    public:
        friend ostream& operator<<(ostream& os, const Object& obj) {
            os << obj.toString();
            return os;
        }

    protected:
        virtual string toString() const { return "??"; };
    };

}