module;
#include "z0/libraries.h"

export module Z0:GWindow;

import :Constants;
import :Object;
import :Rect;
import :Font;
import :Application;
import :GStyle;
import :GWidget;

export namespace z0 {
    class GManager;

    /**
     * A UI window displayed inside the rendering window
     */
    class GWindow : public Object {
    public:
        /**
         * Which GWindow borders can be used to resize the window
         */
        enum ResizeableBorder {
            RESIZEABLE_NONE = 0b0000,
            RESIZEABLE_LEFT = 0b0001,
            RESIZEABLE_RIGHT = 0b0010,
            RESIZEABLE_TOP = 0b0100,
            RESIZEABLE_BOTTOM = 0b1000,
        };

        /**
         * Creates a window with a given position & size
         */
        explicit GWindow(Rect rect);
        virtual ~GWindow() = default;

        /**
         * Sets the borders that can be used to resize the window
         */
        void setResizeableBorders(uint32_t borders) { resizeableBorders = borders; }

        /**
         * Returns the borders that can be used to resize the window
         */
        [[nodiscard]] uint32_t getResizeableBorders() { return resizeableBorders; }

        /** Returns the current style layout or nullptr */
        shared_ptr<GStyle> getStyle() const;

        /** Sets the current style layout. If nullptr, install a default layout */
        void setStyle(shared_ptr<GStyle>);

        /** Returns the main widget .
            This is the widget that covers the entire window and is the parent
            of all the widgets in the window. */
        [[nodiscard]] GWidget& getWidget() const;

        /** Sets the main widget with optional resource string.
            Call SetLayout(nullptr) if no layout have been set previously */
        GWidget& setWidget(shared_ptr<GWidget>  = nullptr, const string& = "", float = 0);

        /** Changes the focus */
        void setFocusedWidget(const shared_ptr<GWidget>&);

        /** Returns the width of the client area */
        [[nodiscard]] float getWidth() const { return rect.width; };

        /** Returns the height of the client area */
        [[nodiscard]] float getHeight() const { return rect.height; };

        /** Sets the client area position & size */
        void setRect(const Rect& rect);

        /** Sets the width of the client area */
        void setWidth(float w);

        /** Sets the height of the client area */
        void setHeight(float h);

        /** Sets the position of the window, bottom-left */
        void setPos(float x, float y);

        /** Sets the position of the window, bottom-left */
        void setPos(vec2 pos);

        /** Sets the X position of the window, bottom-left */
        void setX(float);

        /** Sets the Y position of the window, bottom-left */
        void setY(float);

        /** Returns the size & position of the widget */
        [[nodiscard]] const Rect& getRect() const { return rect; };

        /** Returns true if window is currently visible */
        [[nodiscard]] inline bool isVisible() const { return visible; }

        /** Sets the window visibility. The change will be effective at the start of the next frame */
        void setVisible(bool);

        /** Hides the window. The change will be effective at the start of the next frame : it needs to be called before adding the window to the manager if you want the window to be hidden at startup */
        void hide();

        /** Shows the window. The change will be effective at the start of the next frame */
        void show();

        /** Sets the alpha value for transparency */
        void setTransparency(float);

        /** Event called after window creation (by the window manager) */
        virtual void onCreate() {
        };

        /** Event called after window destruction (by the window manager) */
        virtual void onDestroy() {
        };

        /** Event called when (before) the window manager need to show the window */
        virtual void onShow() {
        };

        /** Event called when (after) the window manager need to hide the window */
        virtual void onHide() {
        };

        /** Event called after a size change */
        virtual void onResize() {
        };

        /** Event called after a position change */
        virtual void onMove() {
        };

        /** Event called when a key was pressed */
        virtual bool onKeyDown(Key) { return false; };

        /** Event called when a key was released */
        virtual bool onKeyUp(Key) { return false; };

        /** Event called when a mouse button was pressed inside the window */
        virtual bool onMouseDown(MouseButton button, float x, float y) { return false; };

        /** Event called when a mouse button was released inside the window */
        virtual bool onMouseUp(MouseButton button, float x, float y) { return false; };

        /** Event called when mouse is moved above the window client area */
        virtual bool onMouseMove(uint32_t buttonsState, float x, float y) { return false; };

        /** Event called when the window got the keyboard focus */
        virtual void onGotFocus() {
        };

        /** Event called when the window lost the keyboard focus */
        virtual void onLostFocus() {
        };

        /**
         * Sets the minimum size of the window (default to {2.0f, 2.0f})
         */
        void setMinimumSize(float width, float height);

        /**
         * Sets the maximum size of the window (default to VECTOR_SCALE)
         */
        void setMaximumSize(float width, float height);

        /**
         * Returns the minimum width of the window
         */
        [[nodiscard]] inline float getMinimumWidth() { return minWidth; }

        /**
         * Returns the minimum height of the window
         */
        [[nodiscard]] inline float getMinimumHeight() { return minHeight; }

        /**
         * Returns the maximum width of the window
         */
        [[nodiscard]] inline float getMaximumWidth() { return maxWidth; }

        /**
         * Returns the maximum height of the window
         */
        [[nodiscard]] inline float getMaximumHeight() { return maxHeight; }

        /**
         * Returns the default font loaded at startup
         */
        [[nodiscard]] shared_ptr<Font>& getDefaultFont();

        void refresh();

    protected:
        Application& app();

    private:
        Rect rect;
        float minWidth{2.0f};
        float minHeight{2.0f};
        float maxWidth{VECTOR_SCALE.x};
        float maxHeight{VECTOR_SCALE.y};
        GManager* windowManager{nullptr};
        bool visible{true};
        bool visibilityChanged{false};
        bool visibilityChange{false};
        shared_ptr<GStyle> layout{nullptr};
        shared_ptr<GWidget> widget{nullptr};
        GWidget* focusedWidget{nullptr};
        float transparency{1.0};
        uint32_t resizeableBorders{RESIZEABLE_NONE};

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
        bool eventMouseMove(uint32_t, float, float);
        void eventGotFocus();
        void eventLostFocus();
    };
}
