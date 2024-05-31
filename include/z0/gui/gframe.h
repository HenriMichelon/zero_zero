#pragma once

namespace z0 {

    // A rectangular frame with an optional title
    class GFrame: public GPanel {
    public:
        // Create a GFrame widget with an optional title
        GFrame(const string& = "");

        // Return the current title of the widget
        const string& getText() const { return text; }

        // Change the title of the widget
        void setText(const string&);

    private:
        string text;
    };
}
