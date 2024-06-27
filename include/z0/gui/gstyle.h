#pragma once

namespace z0 {

    class GWidget;
    class VectorRenderer;

    /**
     * Widget drawing base class
     */
    class GStyle: public Object
    {
    public:
        GStyle();
        virtual ~GStyle() = default;

        /* Create a new layout.
          	string	: layout name
           	nullptr on error (unknown layout)
        */
        [[nodiscard]] static shared_ptr<GStyle> create(const string& = "vector");

        /* Create a resource from a resource description string.
          	string : string that describe the resources of a widget
           	NEVER return nullptr
        */
        virtual void addResource(GWidget&, const string&) = 0;

        /* Set a layout specific option
            	string	: option name
            	string	: value
        */
        virtual void setOption(const string&, const string&);

        /* Read a layout specific option
            	string	: option name
             option value
        */
        [[nodiscard]] string getOption(const string&);

        /* Draw a widget.
          	GWidget	: widget to draw
          	GResource : resources used for drawing this widget
          	bool : TRUE = before drawing children, FALSE = after
        */
        virtual void draw(const GWidget&, GResource&, VectorRenderer&, bool) const = 0;


        /* Resize a widget.
          	GWidget	: widget to draw
          	GResource : resources used for resizing this widget
        */
        virtual void resize(GWidget&, Rect&, GResource&) = 0;


        /* Return the default font for the layout.
          	Font	: font to use for the layout
         */
        [[nodiscard]] shared_ptr<Font> getFont() const { return font; };

    protected:
        shared_ptr<Font>	font;
        virtual bool init() = 0;
        virtual void updateOptions() = 0;

    private:
        class GLayoutOption
        {
        public:
            string name;
            string value;
            GLayoutOption(string N): name(std::move(N)) {};
        };

        list<shared_ptr<GLayoutOption>> options;
    };

}