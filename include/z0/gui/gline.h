#pragma once

#include "z0/gui/gwidget.h"

namespace z0 {

    class GLine: public GWidget {
    public:
        enum LineStyle {
            HORIZ,
            VERT
        };

        explicit GLine(LineStyle = HORIZ);

        LineStyle getStyle() const { return style; };
        void setStyle(LineStyle);

    private:
        LineStyle style;
    };

    class GHLine: public GLine { public: GHLine(): GLine(HORIZ) {} };
    class GVLine: public GLine { public: GVLine(): GLine(VERT) {} };


}
