module;
#include "z0/libraries.h"

export module Z0:GResource;

import :Object;

namespace z0 {

    /**
     * Super class for style resources descriptions
     */
    class GResource: public Object {
    public:
        explicit GResource(string R): res(std::move(R)) {};
        virtual ~GResource() = default;

        [[nodiscard]] const string& Resource() const { return res; };

    private:
        string res;
    };

}