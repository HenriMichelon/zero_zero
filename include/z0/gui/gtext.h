#pragma once

namespace z0 {

    // A widget to display a line of text
    class GText: public GPanel {
    public:
        GText(string);

        const string& getText() const { return text; }
        void setText(const string&);
        void setTextColor(Color c);
        const Color getTextColor() const { return textColor; }

        // get automatic sizing flag
        bool isAutoSize() const { return autoSize; };

        // set automatic sizing flag
        //	bool	: TRUE = auto size when text is changed
        void setAutoSize(bool);

        void computeSize();

    protected:
        string	text;
        Color   textColor;

        virtual void eventCreate();

    private:
        bool	autoSize{true};
    };

}
