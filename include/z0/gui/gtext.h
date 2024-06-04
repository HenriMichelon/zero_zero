#pragma once

namespace z0 {

    // A widget to display a line of text
    class GText: public GWidget {
    public:
        explicit GText(string);

        const string& getText() const { return text; }
        void setText(const string&);
        void setTextColor(Color c);
        Color getTextColor() const { return textColor; }
        void setSize(float, float) override;

    private:
        string	text;
        Color   textColor;

        void computeSize();
        void eventCreate() override;

    };

}
