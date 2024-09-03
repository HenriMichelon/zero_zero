module;

export module Z0:GPanel;

import :GWidget;

export namespace z0 {

    /**
     * A rectangular widget with only a background (no borders)
     */
    class GPanel: public GWidget {
    public:
        GPanel(): GWidget(PANEL) {};

    protected:
        explicit GPanel(const Type T): GWidget(T) {};
    };

}