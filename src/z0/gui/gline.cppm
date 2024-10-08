module;

export module z0:GLine;

import :GWidget;

export namespace z0 {
    /**
     * An horizontal or vertical line
     */
    class GLine : public GWidget {
    public:
        enum LineStyle {
            HORIZ,
            VERT
        };

        explicit GLine(const LineStyle K = HORIZ): GWidget{LINE}, style{K} {
            allowChildren = false;
        }

        [[nodiscard]] LineStyle getStyle() const { return style; };

        void setStyle(const LineStyle K) {
            if (style != K) {
                style = K;
                resizeChildren();
                refresh();
            }
        }

    private:
        LineStyle style;
    };

    class GHLine : public GLine {
    public:
        GHLine(): GLine(HORIZ) {
        }
    };

    class GVLine : public GLine {
    public:
        GVLine(): GLine(VERT) {
        }
    };
}
