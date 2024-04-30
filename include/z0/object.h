#pragma once

#include "z0/tools.h"

namespace z0 {

    class Object {
    public:
        friend std::ostream& operator<<(std::ostream& os, const Object& obj) {
            os << obj.toString();
            return os;
        }

    protected:
        [[nodiscard]] virtual string toString() const { return "??"; };
    };

}