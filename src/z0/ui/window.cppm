/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Window;

import z0.Constants;
import z0.resources.Font;
import z0.Object;

import z0.ui.Rect;
import z0.ui.Style;
import z0.ui.Widget;

export namespace z0::ui {

    /**
     * %A virtual UI Window displayed inside the rendering Window. All UI widgets must belong to a UI window.
     */
    class Window : public Object {
    public:
        /**
         * Which Window borders can be used to resize the Window
         */
        enum ResizeableBorder {
            RESIZEABLE_NONE   = 0b0000,
            RESIZEABLE_LEFT   = 0b0001,
            RESIZEABLE_RIGHT  = 0b0010,
            RESIZEABLE_TOP    = 0b0100,
            RESIZEABLE_BOTTOM = 0b1000,
        };

        /**
         * Creates a virtual UI window with a given position & size
         */
        explicit Window(const Rect& rect);

        /**
         * Sets the borders that can be used to resize the Window
         */
        inline void setResizeableBorders(const uint32_t borders) { resizeableBorders = borders; }

        /**
         * Returns the borders that can be used to resize the Window
         */
        [[nodiscard]] inline uint32_t getResizeableBorders() const { return resizeableBorders; }

        /** Returns the current style layout or null */
        [[nodiscard]] shared_ptr<Style> getStyle() const;

        /** Sets the current style layout. If `null`, installs a default layout */
        void setStyle(const shared_ptr<Style>& style);

        /** Returns the main widget .
            This is the widget that covers the entire Window and is the parent
            of all the widgets in the Window. */
        [[nodiscard]] Widget &getWidget() const;

        /** Sets the main widget with optional resource string.
            Call SetLayout(nullptr) if no layout have been set previously
            \param child	: child widget to add
            \param resources: placement
            \param padding : new widget padding
        */
        void setWidget(shared_ptr<Widget> child = nullptr, const string & resources = "", float padding = 0);

        /** Adds a child widget. Children widgets will be destroyed on parent destruction.
              \param child	    : child widget to add
              \param alignment  : placement
              \param resource	: resource string
              \param overlap    : overlap widget on top of other widgets
       */
        template<typename T>
        auto add(const shared_ptr<T> &child,
                        const Widget::AlignmentType alignment,
                        const string & resource = "",
                        const bool overlap = false) const {
            return getWidget().add(child, alignment, resource, overlap);
        }

        /** Removes a child widget */
        inline void remove(const shared_ptr<Widget>& child) const {
            getWidget().remove(child);
        }

        /** Changes the focus */
        void setFocusedWidget(const shared_ptr<Widget> &);

        /** Returns the width of the client area */
        [[nodiscard]] inline float getWidth() const { return rect.width; }

        /** Returns the height of the client area */
        [[nodiscard]] inline float getHeight() const { return rect.height; }

        /** Sets the client area position & size */
        void setRect(const Rect &newRect);

        /** Sets the width of the client area */
        void setWidth(float width);

        /** Sets the height of the client area */
        void setHeight(float height);

        /** Sets the position of the Window, bottom-left */
        void setPos(float x, float y);

        /** Sets the position of the Window, bottom-left */
        void setPos(vec2 pos);

        /** Sets the X position of the Window, bottom-left */
        void setX(float x);

        /** Sets the Y position of the Window, bottom-left */
        void setY(float y);

        /** Returns the size & position of the widget */
        [[nodiscard]] inline const Rect &getRect() const { return rect; }

        /** Returns true if Window is currently visible */
        [[nodiscard]] inline bool isVisible() const { return visible; }

        /** Sets the Window visibility. The change will be effective at the start of the next frame */
        void setVisible(bool isVisible);

        /** Hides the Window. The change will be effective at the start of the next frame : it needs to be called before adding the Window to the manager if you want the Window to be hidden at startup */
        void hide();

        /** Shows the Window. The change will be effective at the start of the next frame */
        void show();

        /** Sets the alpha value for transparency */
        void setTransparency(float);

        /** Event called after Window creation (by the Window manager) */
        virtual void onCreate() {
        }

        /** Event called after Window destruction (by the Window manager) */
        virtual void onDestroy() {
        }

        /** Event called when (before) the Window manager need to show the Window */
        virtual void onShow() {
        }

        /** Event called when (after) the Window manager need to hide the Window */
        virtual void onHide() {
        }

        /** Event called after a size change */
        virtual void onResize() {
        }

        /** Event called after a position change */
        virtual void onMove() {
        }

        /** Event called when a key was pressed */
        virtual inline bool onKeyDown(Key key) { return false; }

        /** Event called when a key was released */
        virtual inline bool onKeyUp(Key key) { return false; }

        /** Event called when a mouse button was pressed inside the Window */
        virtual inline bool onMouseDown(MouseButton button, float x, float y) { return false; }

        /** Event called when a mouse button was released inside the Window */
        virtual inline bool onMouseUp(MouseButton button, float x, float y) { return false; }

        /** Event called when mouse is moved above the Window client area */
        virtual inline bool onMouseMove(uint32_t buttonsState, float x, float y) { return false; }

        /** Event called when the Window got the keyboard focus */
        virtual void onGotFocus() {
        }

        /** Event called when the Window lost the keyboard focus */
        virtual void onLostFocus() {
        }

        /**
         * Sets the minimum size of the Window (default to {2.0f, 2.0f})
         */
        void setMinimumSize(float width, float height);

        /**
         * Sets the maximum size of the Window (default to VECTOR_SCALE)
         */
        void setMaximumSize(float width, float height);

        /**
         * Returns the minimum width of the Window
         */
        [[nodiscard]] inline float getMinimumWidth() const { return minWidth; }

        /**
         * Returns the minimum height of the Window
         */
        [[nodiscard]] inline float getMinimumHeight() const { return minHeight; }

        /**
         * Returns the maximum width of the Window
         */
        [[nodiscard]] inline float getMaximumWidth() const { return maxWidth; }

        /**
         * Returns the maximum height of the Window
         */
        [[nodiscard]] inline float getMaximumHeight() const { return maxHeight; }

        /**
         * Returns the default font loaded at startup
         */
        [[nodiscard]] Font& getDefaultFont() const;

        inline auto& getDefaultTextColor() const { return defaultTextColor; }

        inline auto setDefaultTextColor(const vec4 color) { return defaultTextColor = color; }

        void refresh() const;

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

        void draw() const;

        void *              windowManager{nullptr};
        bool                visibilityChanged{false};
        bool                visible{true};
        bool                visibilityChange{false};

    private:
        Rect                rect;
        float               minWidth{2.0f};
        float               minHeight{2.0f};
        float               maxWidth{VECTOR_SCALE.x};
        float               maxHeight{VECTOR_SCALE.y};
        shared_ptr<Style>   layout{nullptr};
        shared_ptr<Widget>  widget{nullptr};
        Widget *            focusedWidget{nullptr};
        float               transparency{1.0};
        vec4                defaultTextColor{0.0f, 0.0f, 0.0f, 1.0f};
        uint32_t            resizeableBorders{RESIZEABLE_NONE};

        void unFreeze(const shared_ptr<Widget> &);
    };
}

