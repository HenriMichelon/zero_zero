#pragma once

namespace z0 {

    // An horizontal or vertical line
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
