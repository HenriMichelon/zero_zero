#pragma once

namespace z0 {

    /**
     * A rectangular widget with only a background (no borders)
     */
    class GPanel: public GWidget {
    public:
        GPanel(): GWidget(PANEL) {};

    protected:
        explicit GPanel(Type T): GWidget(T) {};
    };

}