#pragma once

namespace z0 {

    class GManager;

    // An UI window (inside the application window)
    class GWindow: public GEventHandler {
    public:
        explicit GWindow(Rect rect);
        virtual ~GWindow() = default;

        /* Return the current layout or nullptr */
        shared_ptr<GStyle> getLayout() const;

        /* Set the current layout.
            If nullptr, install a default layout */
        void setLayout(shared_ptr<GStyle>);

        /* Return the main widget .
            This is widget that cover the entire window and is the parent
            of all the widgets in the window. */
        GWidget& getWidget();

        /* Set the main widget with optional resource string.
            Call SetLayout(nullptr) if no layout have been set previously */
        GWidget& setWidget(shared_ptr<GWidget> = nullptr, const string& = "", float = 0);

        void setFocusedWidget(const shared_ptr<GWidget>&);

        /* Return the width of the client area */
        float getWidth() const { return rect.width; };

        /* Return the height of the client area */
        float getHeight() const { return rect.height; };

        /* Set the width of the client area */
        void setWidth(float w);

        /* Set the height of the client area */
        void setHeight(float h);

        /* Set the position of the window, bottom-left */
        void setPos(float x, float y);

        void setX(float);
        void setY(float);

        /* Return the size & position of the widget */
        const Rect& getRect() const { return rect; };

        /* \return TRUE if window is currently visible */
        bool isVisible() const { return visible; }

        /* set the window visibility */
        void setVisible(bool);

        /* hide the window */
        void hide();

        /* show the window */
        void show();

        void setTransparency(float);

        /* Event called after window creation (by the window manager) */
        virtual void onCreate() {};

        /* Event called after window destruction (by the window manager) */
        virtual void onDestroy() {};

        /* Event called when (before) the window manager need to show the window */
        virtual void onShow() {};

        /* Event called when (after) the window manager need to hide the window */
        virtual void onHide() {};

        /* Event called after a size change */
        virtual void onResize() {};

        /* Event called after a position change */
        virtual void onMove() {};

        /* Event called when a key was pressed */
        virtual bool onKeyDown(Key) { return false; };

        /* Event called when a key was released */
        virtual bool onKeyUp(Key) { return false; };

        /* Event called when a mouse button was pressed inside the window */
        virtual bool onMouseDown(MouseButton, float, float) { return false; };

        /* Event called when a mouse button was released inside the window */
        virtual bool onMouseUp(MouseButton, float, float) { return false; };

        /* Event called when mouse is moved above the window client area */
        virtual bool onMouseMove(MouseButton, float, float) { return false; };

        /* Event called when the window got the keyboard focus */
        virtual void onGotFocus() {};

        /* Event called when the window lost the keyboard focus */
        virtual void onLostFocus() {};

        shared_ptr<Font>& getDefaultFont() const;
        void refresh();

    protected:
        Application& app();

    private:
        Rect                rect;
        GManager*           windowManager{nullptr};
        bool                visible{true};
        shared_ptr<GStyle>  layout{nullptr};
        shared_ptr<GWidget> widget{nullptr};
        GWidget*            focusedWidget{nullptr};
        float               transparency{1.0};

        void unFreeze(shared_ptr<GWidget>&);
        void draw() const;

        friend class GManager;

        void eventCreate();
        void eventDestroy();
        void eventShow();
        void eventResize();
        void eventMove();
        void eventHide();
        bool eventKeybDown(Key);
        bool eventKeybUp(Key);
        bool eventMouseDown(MouseButton, float, float);
        bool eventMouseUp(MouseButton, float, float);
        bool eventMouseMove(MouseButton, float, float);
        void eventGotFocus();
        void eventLostFocus();
    };

}