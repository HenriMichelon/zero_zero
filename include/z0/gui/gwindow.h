#pragma once

#include "z0/color.h"
#include "z0/gui/glayout.h"

namespace z0 {

    class GManager;

    class GWindow: public Object {
    public:
        explicit GWindow(GRect rect);
        virtual ~GWindow() = default;

        /*! Return the current layout or nullptr */
        shared_ptr<GLayout> getLayout() const;

        /*! Set the current layout.
            If nullptr, install a default layout */
        void setLayout(shared_ptr<GLayout>);

        /*! Return the main widget .
            This is widget that cover the entire window and is the parent
            of all the widgets in the window. */
        GWidget& getWidget();

        /*! Set the main widget with optional resource string.
            Call SetLayout(nullptr) if no layout have been set previously */
        GWidget& setWidget(shared_ptr<GWidget> = nullptr, const string& = "", int32_t = 0);

        void setFocusedWidget(const shared_ptr<GWidget>&);

        /*! Return the y position of the client area */
        int32_t getTop() const { return rect.top; }

        /*! Return the x position of the client area */
        int32_t getLeft() const { return rect.left; };

        /*! Return the width of the client area */
        uint32_t getWidth() const { return rect.width; };

        /*! Return the height of the client area */
        uint32_t getHeight() const { return rect.height; };

        /*! Set the width of the client area */
        void setWidth(uint32_t w);

        /*! Set the height of the client area */
        void setHeight(uint32_t h);

        /*! Set the position of the window */
        void setPos(int32_t l, int32_t t);

        /*! Return the size & position of the widget */
        const GRect& getRect() const { return rect; };

        /*! Change the window default background color */
        void setBgColor(Color c) { bgColor = c; };

        Color getBgColor() { return bgColor; }

        /*! \return TRUE if window is currently visible */
        bool isVisible() const { return visible; }

        /*! Event called after window creation (by the window manager) */
        inline virtual void onCreate() {};

        /*! Event called after window destruction (by the window manager) */
        inline virtual void onDestroy() {};

        /*! Event called when (before) the window manager need to show the window */
        inline virtual void onShow() {};

        /*! Event called when (after) the window manager need to hide the window */
        inline virtual void onHide() {};

        /*! Event called when a key was pressed */
        inline virtual void onKeybDown(Key) {};

        /*! Event called when a key was released */
        inline virtual void onKeybUp(Key) {};

        /*! Event called when a mouse button was pressed */
        inline virtual void onMouseDown(MouseButton, int32_t, int32_t) {};

        /*! Event called when a mouse button was released */
        inline virtual void onMouseUp(MouseButton, int32_t, int32_t) {};

        /*! Event called when mouse is moved above the window */
        inline virtual void onMouseMove(MouseButton, int32_t, int32_t) {};

        /*! Event called when the window got the keyboard focus */
        inline virtual void onGotFocus() {};

        /*! Event called when the window lost the keyboard focus */
        inline virtual void onLostFocus() {};

        /*! Refresh the entire content of the window and all the widgets */
        void refresh();

    private:
        GManager*           windowManager{nullptr};
        GRect               rect;
        bool                visible{true};
        Color               bgColor{0.5,0.5,0.5, 1.0f};
        bool	            freezed{false};
        shared_ptr<GLayout> layout{nullptr};
        shared_ptr<GWidget> widget{nullptr};
        GWidget*            focusedWidget{nullptr};

        void unFreeze(shared_ptr<GWidget>&);

        friend class GManager;

        void eventCreate();
        void eventDestroy();
        void eventShow();
        void eventHide();
        void eventKeybDown(Key);
        void eventKeybUp(Key);
        void eventMouseDown(MouseButton, int32_t, int32_t);
        void eventMouseUp(MouseButton, int32_t, int32_t);
        void eventMouseMove(MouseButton, int32_t, int32_t);
        void eventGotFocus();
        void eventLostFocus();
    };

}