#pragma once

namespace z0 {

    class GWindow;

    /**
     * Base class for all UI widgets
     */
    class GWidget: public Object {
    public:
        //! Widget type
        enum Type {
            //! transparent widget
            WIDGET,				
            //! rectangular widget with only a background
            PANEL,
            //! rectangular widget with a border and a background
            BOX,				
            //! An horizontal or vertical line
            LINE,				
            //! A box with a title
            FRAME,				
            //! A push button
            BUTTON,				
            //! A two states button
            TOGGLEBUTTON,	
            //! A single line of text	
            TEXT,	
            //! An editable single line of text	
            TEXTEDIT,
            //! A scroll bar. with min, max & pos			
            SCROLLBAR,
            //! Tree of GWidget
            TREEVIEW,
            //ARROW,				// up, down, left or right directed arrows
            //CHECKMARK,			// Cross or check mark drawing
            //CHECKBUTTON,		// Button with a checkmark inside
            //RADIOBUTTON,		// two states radio box
            //TEXTEDIT,			// single line text edition field
            //MEMOEDIT,			// multi lines text edition field
            //UPDOWN,				// up & down (or left & right) buttons
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
            //TOOLTIPS,
            //STATUSPANEL,
            //SPLITTER,
            //GRID,
            //GRIDCELL,
            //TABBUTTON,			// button for TABPAGE
            //TABS,
            //SELECTION
        };


        //! Widget placement (relative to the parent widget)
        enum AlignmentType {
            NONE,
            //! The child widget is centered and resized to the parent content size
            FILL,
            //! The child widget is centered (and take all the parent content size)
            CENTER,
            //! The child widget is horizontaly centered
            HCENTER,
            //! The child widget is verticaly centered
            VCENTER,
            //! The children are stack on the top 
            TOP,
            //! The children are stack on the bottom
            BOTTOM,
            //! The children are stack on the left
            LEFT,
            //! The children are stack on the right
            RIGHT,
            //! The children are stack on the top and horizontaly centered
            TOPCENTER,
            //! The children are stack on the bottom and horizontaly centered
            BOTTOMCENTER,
            //! The children are stack on the left and vertically centered
            LEFTCENTER,
            //! The children are stack on the right and vertically centered
            RIGHTCENTER,
            //! The children are stack on the top and left aligned
            TOPLEFT,
            //! The children are stack on the bottom and left aligned
            BOTTOMLEFT,
            //! The children are stack on the bottom and right aligned
            BOTTOMRIGHT,
            //! The children are stack on the top and right aligned
            TOPRIGHT,
            //! The children are stack on the left then on the top
            LEFTTOP,
            //! The children are stack on the left then on the bottom
            LEFTBOTTOM,
            //! The children are stack on the right then on the bottom
            RIGHTBOTTOM,
            //! The children are stack on the right then on the top
            RIGHTTOP,
            //!
            CORNERTOPLEFT,
            //!
            CORNERTOPRIGHT,
            //!
            CORNERBOTTOMLEFT,
            //!
            CORNERBOTTOMRIGHT
        };

        /** Creates a widget of a particular type */
        explicit GWidget(Type = WIDGET);

        virtual ~GWidget() = default;

        /** Returns the type of the widget */
        [[nodiscard]] Type getType() const;

        /** Returns true if the widget is visible */
        [[nodiscard]] bool isVisible() const;

        /** Shows or hides the widget */
        void show(bool = true);

        /** Returns true is the widget is reactive to user action (mouse & keyboard) */
        [[nodiscard]] bool isEnabled() const;

        /** Enables or disable widget reaction */
        void enable(bool = true);

        /** Moves the widget to a particular position. */
        void setPos(float x, float y);

        /** Returns the width of the widget, in pixels */
        [[nodiscard]] float getWidth() const { return rect.width; };

        /** Returns the height of the widget, in pixels */
        [[nodiscard]] float getHeight() const { return rect.height; };

        /** Resizes the widget */
        virtual void setSize(float width, float height);

        /** Returns size size & position of the widget */
        [[nodiscard]] const Rect& getRect() const;

        /** Changes the size & position of the widget */
        void setRect(float x, float y, float width, float height);

        /** Changes the size & position of the widget */
        void setRect(const Rect&);

        /** Returns the current widget placement */
        [[nodiscard]] AlignmentType getAlignment() const;

        /** Sets the widget placement. Calling this method involve 
            redrawing the parent widget & resizing all the children widgets */
        void setAlignment(AlignmentType);

        /** Returns the current font of the widget */
        [[nodiscard]] shared_ptr<Font>& getFont();

        /** Sets the current font of the widget */
        void setFont(const shared_ptr<Font>&);

        /** Returns true if the widget have keyboard focus */
        [[nodiscard]] bool isFocused() const;

        /** Returns the parent widget, or nullptr */
        [[nodiscard]] shared_ptr<GWidget> getParent() const;

        /** Adds a child widget.
              Childs widget will be destroyed on parent destruction.
            	@param GWidget	: child widget to add
            	@param AlignementType	: placement
            	@Param string	: resource string
            	@Param float	: default padding
        */
        virtual shared_ptr<GWidget> add(shared_ptr<GWidget>, AlignmentType, string = "", float = 0);

        /** Removes a child widget */
        virtual void remove(shared_ptr<GWidget>&);

        /** Removes all children widgets recusivly */
        virtual void removeAll();

        /** Changes children padding (space between children) */
        void setPadding(float);

        /** Returns current children padding (space between children) */
        [[nodiscard]] float getPadding() const;

        [[nodiscard]] float getVBorder() const;
        [[nodiscard]] float getHBorder() const;
        void setVBorder(float);
        void setHBorder(float);

        /** Returns false if the background is transparent */
        [[nodiscard]] bool isDrawBackground() const;

        /** Sets to true to disable de background drawing */
        void setDrawBackground(bool isTransparent);

        [[nodiscard]] bool isPushed() const;
        [[nodiscard]] bool isPointed() const;
        [[nodiscard]] bool isFreezed() const;
        [[nodiscard]] bool isRedrawOnMouseEvent() const;
        [[nodiscard]] Rect getChildrenRect() const;
        void setFreezed(bool f) { freeze = f; }
        void setPushed(bool p) { pushed = p; }

        /** Force a refresh of the entire widget */
        void refresh();

        /** Changes widget resources. Use with caution ! */
        void setResource(shared_ptr<GResource>);

        /** Return the user defined group index */
        [[nodiscard]] uint32_t getGroupIndex() const;

        /** Set the user defined group index */
        void setGroupIndex(int32_t);

        /** Returns the user data */
        void* getUserData() const;

        /** set user data */
        void setUserData(void*);

        /** Return the transparency alpha value */
        [[nodiscard]] inline const float getTransparency() const { return transparency; }

        /** Changes the transpency alpha value */
        void setTransparency(float alpha);
        
        void resizeChildren();

        void _setRedrawOnMouseEvent(bool r) { redrawOnMouseEvent = r; }
        void _setMoveChildrenOnPush(bool r) { moveChildrenOnPush = r; }
        [[nodiscard]] virtual list<shared_ptr<GWidget>>& _getChildren() { return children; };

    protected:
        Rect			          rect;
        Rect			          defaultRect;
        float			          hborder{0};
        float			          vborder{0};
        float		              padding{0};
        bool			          focused{false};
        bool			          allowFocus{false};
        bool			          allowChildren{true};
        bool			          drawBackground{true};
        bool			          moveChildrenOnPush{false};
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

        void allowingFocus(bool = true);
        Application& app();

        [[nodiscard]] inline virtual Rect _getDefaultRect() { return defaultRect; };

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
        virtual bool eventMouseMove(uint32_t, float, float);
        virtual void eventGotFocus();
        virtual void eventLostFocus();
        
        virtual void _init(GWidget&, AlignmentType, const string&, float);

    private:
        bool		     pushed{false};
        bool		     pointed{false};
        bool		     freeze{true};
        bool		     enabled{true};
        bool		     visible{true};
        shared_ptr<Font> font{nullptr};
        void*		     userData{nullptr};
        int32_t		     groupIndex{0};
        Rect		     childrenRect;

        friend class GWindow;

        GWidget* setNextFocus();
        GWidget* setFocus(bool = true);

        void _draw(VectorRenderer&) const;
   };

}