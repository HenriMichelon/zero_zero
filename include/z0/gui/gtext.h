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

       /*  void setSize(float, float) override;
        void setRect(const Rect&);
        void setRect(float, float, float, float); */

         /**
         * Returns the size (in screen units, VECTOR_SCALE ratio applied) for the text.
         */
        void getSize(float&width, float&height);

        void setSize(float width, float height) override;

    protected:
        [[nodiscard]] Rect _getDefaultRect() override;

    private:
        string	text;
        Color   textColor;

        /* void computeSize();*/
        void eventCreate() override;

    };

}
