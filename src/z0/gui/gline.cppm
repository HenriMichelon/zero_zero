module;
#include "z0/modules.h"

export module Z0:GLine;

import :GWidget;

export namespace z0 {

    /**
     * An horizontal or vertical line
     */
    class GLine: public GWidget {
    public:
        enum LineStyle {
            HORIZ,
            VERT
        };

        explicit GLine(LineStyle = HORIZ);

        [[nodiscard]] LineStyle getStyle() const { return style; };
        void setStyle(LineStyle);

    private:
        LineStyle style;
    };

    class GHLine: public GLine { public: GHLine(): GLine(HORIZ) {} };
    class GVLine: public GLine { public: GVLine(): GLine(VERT) {} };

    GLine::GLine(GLine::LineStyle K): GWidget{LINE}, style{K} {
        allowChildren = false;
    }

    void GLine::setStyle(GLine::LineStyle K) {
        if (style != K)	{
            style = K;
            resizeChildren();
            refresh();
        }
    }


}
