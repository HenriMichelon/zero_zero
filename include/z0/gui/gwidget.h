#pragma once

#include "z0/gui/gevent.h"
#include "z0/gui/glayout.h"

namespace z0 {

    class GWindow;

    class GWidget {
    public:
        typedef void (*GEventFunction)(GWidget&, GEvent*);
        
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
            CLIENT,
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
        GWidget(Type = WIDGET);

        virtual ~GWidget() = default;

        //! Return the type of the widget
        Type getType() const;

        //! Return true if the widget is visible
        bool isVisible() const;

        //! Show or hide the widget
        void show(bool = true);

        //! Return true is the widget is reactive to user action (mouse & keyboard)
        bool isEnabled() const;

        //! Enable or disable widget reactino
        void enable(bool = true);

        //! Return the top border position of the widget (in pixels), relative to the parent widget
        uint32_t getTop() const;

        //! Return the left border position of the widget (in pixels), relative to the parent widget
        uint32_t getLeft() const;

        /*! Move the widget to a particular position.
            \param int32_t	: Left position in pixels
            \param int32_t	: top position in pixels
            \param bool	: force redraw 
         */
        void setPos(int32_t, int32_t, bool = true);

        //! Return the width of the widget, in pixels
        uint32_t getWidth() const;

        //! Return the height of the widget, in pixels
        uint32_t getHeight() const;

        /*! Resize the widget
            \param uint32_t	: width in pixels
            \param uint32_t	: height in pixels
            \param bool	: force redraw
        */
        void setSize(uint32_t, uint32_t, bool = true);

        /*! Return size size & position of the widget */
        const GRect& getRect() const;

        /*! Change the size & position of the widget
          \param	int32_t	: left position in pixels
          \param	int32_t	: top position in pixels
          \param	uint32_t	: width in pixels
          \param	uint32_t	: height in pixels
            \param bool	: force redraw
        */
        void setRect(int32_t, int32_t, uint32_t, uint32_t, bool = true);

        /*! Change the size & position of the widget
          \param	GRect	: size & position, all in pixels
          \param	bool	: force redraw
         */
        void setRect(const GRect&, bool = true);

        //! Return the current widget placement
        AlignmentType getAlignment() const;

        /*! Set the widget placement. Calling this method involve 
            redrawing the parent widget & resizing all the childs widgets */
        void setAlignment(AlignmentType);

        /*! Return the current font of the widget */
        Font& getFont();

        //! Set the current font of the widget
        void setFont(shared_ptr<Font>&);

        //! Return true if the widget have keyboard focus
        bool isFocused() const;

        //! Return the parent widget, or nullptr */
        shared_ptr<GWidget> getParent() const;

        /*! Return the list of direct childs widgets.
            Do NOT use this list to add or remove childs widget, 
            use Add(), Drop() & DropAll() instead */
        virtual list<shared_ptr<GWidget>>& getChildren();

        /*! Add a child widget.
              Childs widget will be destroyed on parent destruction.
            \param	GWidget	: child widget to add
            \param	AlignementType	: placement
            \param	string	: resource string
            \param	uint32_t	: default padding
        */
        virtual shared_ptr<GWidget> add(shared_ptr<GWidget>, AlignmentType = NONE, const string& = "", uint32_t = 0);

        /*! Remove a child widget */
        virtual void remove(shared_ptr<GWidget>&);

        /*! Remove all childs widgets recusivly */
        virtual void removeAll();

        /*! Change childs padding (space between childs) */
        void setPadding(int32_t);

        /*! Return current childs padding (space between childs) */
        uint32_t getPadding() const;

        uint32_t getVBorder() const;
        uint32_t getHBorder() const;
        void setVBorder(uint32_t);
        void setHBorder(uint32_t);

        bool getDrawBackground() const;
        void setDrawBackground(bool);

        bool& isPushed();
        bool& isPointed();
        bool& isFreezed();
        bool& isTransparent();
        bool& isRedrawOnMouseEvent();
        bool& isMoveChildsOnPush();
        GRect& getChildrenRect();
        void resizeChildren();

        /*! Force a refresh of the entire widget */
        void refresh();

        /*! Force a refresh of a part of the widget */
        void refresh(const GRect&, bool = true);

        /*! Connect an object method to a event.
          \param	EventType	: event type
          \param	_PTR		: object address
          \param	GEventFunction	: method offset 
        */
        void connect(GEvent::Type, void*, const GEventFunction);

        /*! Call the object method connected to an event, if any.
          \param	EventType	: event to simulate
          \param	GEvent		: event parameter. Note: this pointer is AUTOMATICALLY deleted 
        */
        void call(GEvent::Type, GEvent* = nullptr);

        /*! Simulate a user/system event
          \param	EventType	: event to simulate
        */
        void simulate(GEvent::Type, GEvent* = nullptr);

        /*GPopupMenu* PopupMenu();
        GPopupMenu* SetPopupMenu(GPopupMenu*);
        void ClosePopup();*/

        /*! Return the user defined group index */
        uint32_t getGroupIndex() const;

        /*! Set the user defined group index */
        void setGroupIndex(int32_t);

        /*! Return user data */
        void* getData() const;

        /*! set user data */
        void setData(void*);

        friend class GWindow;

        /*! Return the parent window */
        GWindow& Window();
        
    protected:
        int32_t			mHborder;
        int32_t			mVborder;
        int32_t			mPadding;
        bool			focused;
        bool			allowFocus;
        bool			allowChilds;
        bool			transparent;
        bool			drawBackground;
        bool			moveChildsOnPush;
        bool			moveChildsNow;
        bool			redrawOnMouseEvent;
        bool			redrawOnMouseMove;
        bool			mouseMoveOnFocus;
        GRect			rect;
        shared_ptr<Font>			font;
        GWidget*			parent;
        GWindow*			window;
        Type		    type;
        AlignmentType	alignment;
        shared_ptr<GLayout>			layout;
        shared_ptr<GResource>		resource;

        list<shared_ptr<GWidget>>	childs;

        void maxRect(GRect&, GRect, GRect) const;
        bool clipRect(GRect&, const GRect&, const GRect&) const;
        void allowingFocus(bool = true);

        virtual void eventCreate();
        virtual void eventDestroy();
        virtual void eventDraw(const GRect&, bool);
        virtual void eventShow();
        virtual void eventHide();
        virtual void eventEnable();
        virtual void eventDisable();
        virtual void eventMove(int32_t, int32_t, bool);
        virtual void eventResize(bool);
        virtual Key eventKeybDown(Key);
        virtual Key eventKeybUp(Key);
        virtual shared_ptr<GWidget> eventMouseDown(MouseButton, int32_t, int32_t);
        virtual void eventMouseUp(MouseButton, int32_t, int32_t);
        virtual void eventMouseMove(MouseButton, int32_t, int32_t);
        virtual void eventGotFocus();
        virtual void eventLostFocus();

    private:
        class GEventSlot
        {
        public:
            GWidget		*obj;   // TODO Object ?
            GEventFunction	func;

            GEventSlot(): obj(nullptr), func(nullptr) {};
        };

        bool		mPushed;
        bool		mPointed;
        bool		mFreeze;
        bool		mEnabled;
        bool		mVisible;

        void*		mUserData;
        int32_t		mGroupIndex;
        GRect		mChildsRect;
        GRect		mRefreshRect;
        GEventSlot	slots[GEvent::nbEvents];

        GWidget* setNextFocus();
        GWidget* setFocus(bool = true);

        void flushRefresh(GRect&);
        void reallyDraw(const GRect&);
        void init(GWidget&, AlignmentType, const string&, uint32_t);
    };

}