#pragma once

namespace z0 {

    /**
     * A widget to display a line of text
     */ 
    class GText: public GWidget {
    public:
        explicit GText(string);

        [[nodiscard]] const string& getText() const { return text; }
        void setText(const string&);
        void setTextColor(Color c);
        [[nodiscard]] Color getTextColor() const { return textColor; }

        void setSize(float, float) override;
        void setRect(const Rect&);
        void setRect(float, float, float, float);

    private:
        string	text;
        Color   textColor;

        void computeSize();
        void eventCreate() override;

    };

}
