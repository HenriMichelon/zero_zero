#pragma once

#include "z0/gui/gevent.h"
#include "z0/gui/gstyle.h"

namespace z0 {

    class GWindow;

    class GWidget: public Object {
    public:
        enum Type {
            WIDGET,				// transparent widget
            PANEL,				// rectangular widget with only a background
            BOX,				// rectangular widget with a border and a background
            SCROLLBOX,			// A box with scrollbars
            LINE,				// An horizontal or vertical line
            FRAME,				// A box with a title
            ARROW,				// up, down, left or right directed arrows
            BUTTON,				// rectangular button
            TOGGLEBUTTON,		// two states rectangular button
            CHECKMARK,			// Cross or check mark drawing
            CHECKBUTTON,		// Button with a checkmark inside
            //RADIOBUTTON,		// two states radio box
            TEXT,				// single line text
            TEXTEDIT,			// single line text edition field
            //MEMOEDIT,			// multi lines text edition field
            UPDOWN,				// up & down (or left & right) buttons
            SCROLLBAR,			// scroll bar. with min, max & pos
            TRACKBAR,			// horizontal track bar. with min, max, pos & step
            ROUNDBUTTON,		// round button. with min, max, pos & step
            PROGRESSBAR,		// progression bar. with min, max & pos
            PICTURE,			// Pixmap
            LISTBOX,			// selection list with scrollbar
            //DROPLIST,			// drop down list
            //DROPEDITLIST, 	// drop down list with a TEXTENTRY
            //MENU,
            //POPUPMENU,
            //OUTLINE,
            //TREEVIEW,
            //TOOLTIPS,
            //STATUSPANEL,
            //SPLITTER,
            GRID,
            GRIDCELL,
            TABBUTTON,			// button for TABPAGE
            TABS,
            SELECTION
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

        /*! Move the widget to a particular position.
            \param int32_t	: Left position in pixels
            \param int32_t	: top position in pixels
         */
        void setPos(int32_t, int32_t);

        //! Return the width of the widget, in pixels
        uint32_t getWidth() const { return rect.width; };

        //! Return the height of the widget, in pixels
        uint32_t getHeight() const { return rect.height; };

        /*! Resize the widget
            \param uint32_t	: width in pixels
            \param uint32_t	: height in pixels
        */
        void setSize(uint32_t, uint32_t);

        /*! Return size size & position of the widget */
        const Rect& getRect() const;

        /*! Change the size & position of the widget
          \param	int32_t	: left position in pixels
          \param	int32_t	: bottom position in pixels
          \param	uint32_t	: width in pixels
          \param	uint32_t	: height in pixels
        */
        void setRect(int32_t, int32_t, uint32_t, uint32_t);

        /*! Change the size & position of the widget
          \param	GRect	: size & position, all in pixels
         */
        void setRect(const Rect&);

        //! Return the current widget placement
        AlignmentType getAlignment() const;

        /*! Set the widget placement. Calling this method involve 
            redrawing the parent widget & resizing all the children widgets */
        void setAlignment(AlignmentType);

        /*! Return the current font of the widget */
        shared_ptr<Font>& getFont();

        //! Set the current font of the widget
        void setFont(shared_ptr<Font>&);

        //! Return true if the widget have keyboard focus
        bool isFocused() const;

        //! Return the parent widget, or nullptr */
        shared_ptr<GWidget> getParent() const;

        /*! Return the list of direct children widgets.
            Do NOT use this list to add or remove children widget,
            use Add(), Drop() & DropAll() instead */
        virtual list<shared_ptr<GWidget>>& getChildren() { return children; };

        /*! Add a child widget.
              Childs widget will be destroyed on parent destruction.
            \param	GWidget	: child widget to add
            \param	AlignementType	: placement
            \param	string	: resource string
            \param	uint32_t	: default padding
        */
        virtual shared_ptr<GWidget> add(shared_ptr<GWidget>, AlignmentType, string = "", uint32_t = 0);

        /*! Remove a child widget */
        virtual void remove(shared_ptr<GWidget>&);

        /*! Remove all children widgets recusivly */
        virtual void removeAll();

        /*! Change children padding (space between children) */
        void setPadding(int32_t);

        /*! Return current children padding (space between children) */
        int32_t getPadding() const;

        uint32_t getVBorder() const;
        uint32_t getHBorder() const;
        void setVBorder(uint32_t);
        void setHBorder(uint32_t);

        bool isDrawBackground() const;
        void setDrawBackground(bool);

        bool isPushed() const;
        bool isPointed() const;
        bool isFreezed() const;
        bool isTransparent() const;
        bool isRedrawOnMouseEvent() const;
        bool isMoveChildsOnPush() const;
        Rect getChildrenRect() const;

        void setTransparent(bool t) { transparent = t; }
        void setFreezed(bool f) { freeze = f; }
        void setPushed(bool p) { pushed = p; }

        /*! Force a refresh of the entire widget */
        void refresh();

        /*! Connect an object method to a event.
          \param	EventType	: event type
          \param	_PTR		: object address
          \param	GEventFunction	: method offset 
        */
        void connect(GEvent::Type, void*, GEventFunction);

        /*! Call the object method connected to an event, if any.
          \param	EventType	: event to simulate
          \param	GEvent		: event parameter.
        */
        bool call(GEvent::Type, shared_ptr<GEvent> = nullptr);

        /*! Simulate a user/system event
          \param	EventType	: event to simulate
        */
        void simulate(GEvent::Type,  shared_ptr<GEvent> = nullptr);

        /*! Change widget resources. Use with caution ! */
        void setResource(shared_ptr<GResource>);

        /*! Return the user defined group index */
        uint32_t getGroupIndex() const;

        /*! Set the user defined group index */
        void setGroupIndex(int32_t);

        /*! Return user data */
        void* getData() const;

        /*! set user data */
        void setData(void*);

        friend class GWindow;

        /*! recursively draw the widget and his children */
        void draw(VectorRenderer&) const;
        
    protected:
        int32_t			          hborder{0};
        int32_t			          vborder{0};
        int32_t		              padding{0};
        bool			          focused{false};
        bool			          allowFocus{false};
        bool			          allowChildren{true};
        bool			          transparent;
        bool			          drawBackground{true};
        bool			          moveChildsOnPush{false};
        bool			          moveChildsNow{false};
        bool			          redrawOnMouseEvent{false};
        bool			          redrawOnMouseMove{false};
        bool			          mouseMoveOnFocus{false};
        Rect			          rect;
        GWidget*		          parent{nullptr};
        GWindow*		          window{nullptr};
        Type		              type;
        AlignmentType             alignment{NONE};
        shared_ptr<GStyle>	      layout{nullptr};
        shared_ptr<GResource>     resource;
        list<shared_ptr<GWidget>> children;

        void maxRect(Rect&, Rect, Rect) const;
        bool clipRect(Rect&, const Rect&, const Rect&) const;
        void allowingFocus(bool = true);
        void resizeChildren();

        virtual void eventCreate();
        virtual void eventDestroy();
        virtual void eventShow();
        virtual void eventHide();
        virtual void eventEnable();
        virtual void eventDisable();
        virtual void eventMove(int32_t, int32_t);
        virtual void eventResize();
        virtual bool eventKeybDown(Key);
        virtual bool eventKeybUp(Key);
        virtual bool eventMouseDown(MouseButton, uint32_t, uint32_t);
        virtual bool eventMouseUp(MouseButton, uint32_t, uint32_t);
        virtual bool eventMouseMove(MouseButton, uint32_t, uint32_t);
        virtual void eventGotFocus();
        virtual void eventLostFocus();

    private:
        struct GEventSlot {
            Object*         obj{nullptr};
            GEventFunction	func{nullptr};
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

        void init(GWidget&, AlignmentType, const string&, uint32_t);
    };

}