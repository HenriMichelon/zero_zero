#pragma once

namespace z0 {

    class Object {
    public:
        friend ostream& operator<<(ostream& os, const Object& obj) {
            os << obj.toString();
            return os;
        }

        virtual string toString() const { return "??"; };
    };

}