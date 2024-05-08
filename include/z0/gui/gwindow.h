#pragma once

#include "z0/color.h"
#include "z0/gui/glayout.h"

namespace z0 {

    class GWindow {
    public:
        GWindow();
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
        GWidget& setWidget(shared_ptr<GWidget> = nullptr, const string& = "", uint32_t = 0);

        void setFocusedWidget(shared_ptr<GWidget>);

        /*! Return the width of the client area (not the window) */
        uint32_t getWidth() const;

        /*! Return the height of the client area (not the window) */
        uint32_t getHeight() const;

        /*! Change the window default background color */
        void setBgColor(Color);

        /*! \return TRUE if window is currently visible */
        bool isVisible() const;

        /*! Event called after window creation (by the window manager) */
        inline virtual void onCreate() {};

        /*! Event called before window destruction.
            \return true if the window can be destroyed, FALSE cancel destruction */
        inline virtual bool onQueryDestroy() { return true; };

        /*! Event called after window destruction (by the window manager) */
        inline virtual void onDestroy() {};

        /*! Event called before a part of the window is refreshed */
        inline virtual void onDraw(const GRect&) {};

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

        /*! Start refresing session. All refreshed widgets will be
            recorded */
        void startRefresh();

        /*! End refreshing session. All recorded widgets will be
            refreshed, and window content updated */
        void endRefresh();

        /*! Refresh the entire content of the window and all the widgets */
        void refresh();

    private:
        bool	mFreeze;
        bool	mDestroylayout;
        int32_t	mModalResult;
        GRect	mRefreshrect;
        shared_ptr<GLayout> mLayout;
        shared_ptr<GWidget> mWidget;
        shared_ptr<GWidget> mFocusedWidget;

        void unFreeze(GWidget&);

        virtual void eventCreate();
        virtual void eventDestroy();
        virtual bool eventQueryDestroy();
        virtual void eventDraw(const GRect&);
        virtual void eventShow();
        virtual void eventHide();
        virtual void eventKeybDown(Key);
        virtual void eventKeybUp(Key);
        virtual void eventMouseDown(MouseButton, int32_t, int32_t);
        virtual void eventMouseUp(MouseButton, int32_t, int32_t);
        virtual void eventMouseMove(MouseButton, int32_t, int32_t);
        virtual void eventGotFocus();
        virtual void eventLostFocus();
    };

}