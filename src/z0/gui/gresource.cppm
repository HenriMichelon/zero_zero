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
        explicit GResource(const string& R): res(std::move(R)) {};
        ~GResource() override = default;

        [[nodiscard]] const string& Resource() const { return res; };

    private:
        string res;
    };

}