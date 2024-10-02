module;

export module z0:GBox;

import :GPanel;

namespace z0 {

    /**
     * A rectangular box
     */
    class GBox: public GPanel {
    public:
        GBox(): GPanel {BOX} {}

    protected:
        explicit GBox(const Type T): GPanel{T} {}
    };


}