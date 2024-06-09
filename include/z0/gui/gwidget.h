#pragma once

namespace z0 {

    class GWindow;

    // Super class for all widgets
    class GWidget: public GEventHandler {
    public:
        enum Type {
            WIDGET,				// transparent widget
            PANEL,				// rectangular widget with only a background
            BOX,				// rectangular widget with a border and a background
            SCROLLBOX,			// A box with scrollbars
            LINE,				// An horizontal or vertical line
            FRAME,				// A box with a title
            //ARROW,				// up, down, left or right directed arrows
            BUTTON,				// rectangular button
            TOGGLEBUTTON,		// two states rectangular button
            //CHECKMARK,			// Cross or check mark drawing
            //CHECKBUTTON,		// Button with a checkmark inside
            //RADIOBUTTON,		// two states radio box
            TEXT,				// single line text
            TEXTEDIT,			// single line text edition field
            //MEMOEDIT,			// multi lines text edition field
            //UPDOWN,				// up & down (or left & right) buttons
            //SCROLLBAR,			// scroll bar. with min, max & pos
            //TRACKBAR,			// horizontal track bar. with min, max, pos & step
            //ROUNDBUTTON,		// round button. with min, max, pos & step
            //PROGRESSBAR,		// progression bar. with min, max & pos
            //PICTURE,			// Pixmap
            //LISTBOX,			// selection list with scrollbar
            //DROPLIST,			// drop down list
            //DROPEDITLIST, 	// drop down list with a TEXTENTRY
            //MENU,
            //POPUPMENU,
            //OUTLINE,
            //TREEVIEW,
            //TOOLTIPS,
            //STATUSPANEL,
            //SPLITTER,
            //GRID,
            //GRIDCELL,
            //TABBUTTON,			// button for TABPAGE
            //TABS,
            //SELECTION
        };


        // Widget placement (relative to the parent widget)
        enum AlignmentType {
            NONE,
            FILL,
            CENTER,
            HCENTER,
            VCENTER,
            TOP,
            BOTTOM,
            LEFT,
            RIGHT,
            TOPCENTER,
            BOTTOMCENTER,
            LEFTCENTER,
            RIGHTCENTER,
            TOPLEFT,
            BOTTOMLEFT,
            BOTTOMRIGHT,
            TOPRIGHT,
            LEFTTOP,
            LEFTBOTTOM,
            RIGHTBOTTOM,
            RIGHTTOP,
            CORNERTOPLEFT,
            CORNERTOPRIGHT,
            CORNERBOTTOMLEFT,
            CORNERBOTTOMRIGHT
        };

        //! Create a widget of a particular type
        explicit GWidget(Type = WIDGET);

        virtual ~GWidget() = default;

        //! Return the type of the widget
        Type getType() const;

        //! Return true if the widget is visible
        bool isVisible() const;

        //! Show or hide the widget
        void show(bool = true);

        //! Return true is the widget is reactive to user action (mouse & keyboard)
        bool isEnabled() const;

        //! Enable or disable widget reaction
        void enable(bool = true);

        /* Move the widget to a particular position.
             float	: Left position in pixels
             float	: top position in pixels
         */
        void setPos(float, float);

        //! Return the width of the widget, in pixels
        float getWidth() const { return rect.width; };

        //! Return the height of the widget, in pixels
        float getHeight() const { return rect.height; };

        /* Resize the widget
             float	: width in pixels
             float	: height in pixels
        */
        virtual void setSize(float, float);

        /* Return size size & position of the widget */
        const Rect& getRect() const;

        /* Change the size & position of the widget
          	float	: left position in pixels
          	float	: bottom position in pixels
          	float	: width in pixels
          	float	: height in pixels
        */
        void setRect(float, float, float, float);

        /* Change the size & position of the widget
          	GRect	: size & position, all in pixels
         */
        void setRect(const Rect&);

        //! Return the current widget placement
        AlignmentType getAlignment() const;

        /* Set the widget placement. Calling this method involve 
            redrawing the parent widget & resizing all the children widgets */
        void setAlignment(AlignmentType);

        /* Return the current font of the widget */
        shared_ptr<Font>& getFont();

        //! Set the current font of the widget
        void setFont(const shared_ptr<Font>&);

        //! Return true if the widget have keyboard focus
        bool isFocused() const;

        //! Return the parent widget, or nullptr */
        shared_ptr<GWidget> getParent() const;

        /* Return the list of direct children widgets.
            Do NOT use this list to add or remove children widget,
            use Add(), Drop() & DropAll() instead */
        virtual list<shared_ptr<GWidget>>& getChildren() { return children; };

        /* Add a child widget.
              Childs widget will be destroyed on parent destruction.
            	GWidget	: child widget to add
            	AlignementType	: placement
            	string	: resource string
            	float	: default padding
        */
        virtual shared_ptr<GWidget> add(shared_ptr<GWidget>, AlignmentType, string = "", float = 0);

        /* Remove a child widget */
        virtual void remove(shared_ptr<GWidget>&);

        /* Remove all children widgets recusivly */
        virtual void removeAll();

        /* Change children padding (space between children) */
        void setPadding(float);

        /* Return current children padding (space between children) */
        float getPadding() const;

        float getVBorder() const;
        float getHBorder() const;
        void setVBorder(float);
        void setHBorder(float);

        bool isDrawBackground() const;
        void setDrawBackground(bool);

        bool isPushed() const;
        bool isPointed() const;
        bool isFreezed() const;
        bool isRedrawOnMouseEvent() const;
        Rect getChildrenRect() const;

        void setFreezed(bool f) { freeze = f; }
        void setPushed(bool p) { pushed = p; }

        /* Force a refresh of the entire widget */
        void refresh();

        /* Connect an object method to a event.
          	EventType	: event type
          	_PTR		: object address
          	GEventFunction	: method offset 
        */
        void connect(GEvent::Type, GEventHandler*, GEventFunction);

        /* Call the object method connected to an event, if any.
          	EventType	: event to simulate
          	GEvent		: event parameter.
        */
        bool call(GEvent::Type, shared_ptr<GEvent> = nullptr);

        /* Change widget resources. Use with caution ! */
        void setResource(shared_ptr<GResource>);

        /* Return the user defined group index */
        uint32_t getGroupIndex() const;

        /* Set the user defined group index */
        void setGroupIndex(int32_t);

        /* Return user data */
        void* getData() const;

        /* set user data */
        void setData(void*);

        friend class GWindow;

        /* recursively draw the widget and his children */
        void draw(VectorRenderer&) const;

        const float getTransparency() const { return transparency; }
        void setTransparency(float alpha);

    protected:
        Rect			          rect;
        float			          hborder{0};
        float			          vborder{0};
        float		              padding{0};
        bool			          focused{false};
        bool			          allowFocus{false};
        bool			          allowChildren{true};
        bool			          drawBackground{true};
        bool			          moveChildsOnPush{false};
        bool			          redrawOnMouseEvent{false};
        bool			          redrawOnMouseMove{false};
        bool			          mouseMoveOnFocus{false};
        float                     transparency{1.0f};
        GWidget*		          parent{nullptr};
        GWindow*		          window{nullptr};
        Type		              type;
        AlignmentType             alignment{NONE};
        shared_ptr<GStyle>	      style{nullptr};
        shared_ptr<GResource>     resource;
        list<shared_ptr<GWidget>> children;

        static void maxRect(Rect&, Rect, Rect) ;
        static bool clipRect(Rect&, const Rect&, const Rect&) ;
        void allowingFocus(bool = true);
        void resizeChildren();
        Application& app();

        virtual void eventCreate();
        virtual void eventDestroy();
        virtual void eventShow();
        virtual void eventHide();
        virtual void eventEnable();
        virtual void eventDisable();
        virtual void eventMove(float, float);
        virtual void eventResize();
        virtual bool eventKeybDown(Key);
        virtual bool eventKeybUp(Key);
        virtual bool eventMouseDown(MouseButton, float, float);
        virtual bool eventMouseUp(MouseButton, float, float);
        virtual bool eventMouseMove(MouseButton, float, float);
        virtual void eventGotFocus();
        virtual void eventLostFocus();

    private:
        struct GEventSlot {
            GEventHandler* obj{nullptr};
            GEventFunction func{nullptr};
        };

        bool		     pushed{false};
        bool		     pointed{false};
        bool		     freeze{true};
        bool		     enabled{true};
        bool		     visible{true};
        shared_ptr<Font> font{nullptr};
        void*		     userData{nullptr};
        int32_t		     groupIndex{0};
        Rect		     childrenRect;
        GEventSlot	     slots[GEvent::nbEvents];

        GWidget* setNextFocus();
        GWidget* setFocus(bool = true);

        void init(GWidget&, AlignmentType, const string&, float);
    };

}