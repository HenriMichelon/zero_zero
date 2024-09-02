module;
#include "z0/libraries.h"

export module Z0:GBox;

import :GPanel;

namespace z0 {

    /**
     * A rectangular box
     */
    class GBox: public GPanel {
    public:
        GBox(): GPanel {BOX} {}

    protected:
        GBox(Type T): GPanel{T} {}
    };


}