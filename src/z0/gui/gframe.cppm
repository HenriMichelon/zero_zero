module;
#include "z0/libraries.h"

export module z0:GFrame;

import :Color;
import :GPanel;

export namespace z0 {

    /**
     * A rectangular frame with an optional title
     */
    class GFrame: public GPanel {
    public:
        // Create a GFrame widget with an optional title
        explicit GFrame(const string& STR= ""): GPanel(GWidget::FRAME), text(STR) {
        }

        // Return the current title of the widget
        [[nodiscard]] const string& getText() const { return text; }

        // Change the title of the widget
        void setText(const string& T) {
            text = T;
            resizeChildren();
            refresh();
        }

        void setTextColor(const Color c) { textColor = c; }

        [[nodiscard]] Color getTextColor() const { return textColor; }

    private:
        string  text{};
        Color   textColor;
    };

}
