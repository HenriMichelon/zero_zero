#pragma once

#include "z0/gui/gresource.h"

#include <cstdint>

namespace z0 {

    class GResourceVector: public GResource {
    public:
        enum Style {
            FLAT,
            RAISED,
            LOWERED
        };

        Style		style{FLAT};
        uint32_t	width{0};
        uint32_t	height{0};
        bool		flat{false};

        explicit GResourceVector(const string&);

    private:
        void splitResString(const string&);
    };
    
}