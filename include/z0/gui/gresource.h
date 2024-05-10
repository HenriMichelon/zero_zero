#pragma once

#include <utility>

#include "z0/object.h"

namespace z0 {

    class GResource: public Object {
    public:
        explicit GResource(string R): res(std::move(R)) {};
        virtual ~GResource() = default;

        const string& Resource() const { return res; };

    private:
        string res;
    };

}