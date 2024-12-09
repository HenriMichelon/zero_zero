/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Widget;

import z0.Constants;
import z0.Object;

import z0.resources.Font;

import z0.ui.Rect;
import z0.ui.Resource;

import z0.vulkan.VectorRenderer;

namespace z0 {

    namespace ui {
        /**
         * Base class for all UI widgets
         */
        export class Widget : public Object {
        public:

            //! Widget type
            enum Type {
                //! transparent widget
                WIDGET,
                //! rectangular widget with only a background
                PANEL,
                //! rectangular widget with a border and a background
                BOX,
                //! %A horizontal or vertical line
                LINE,
                //! %A box with a title
                FRAME,
                //! %A push button
                BUTTON,
                //! %A two states button
                TOGGLEBUTTON,
                //! %A single line of text
                TEXT,
                //! An editable single line of text
                TEXTEDIT,
                //! %A scroll bar. with min, max & pos
                SCROLLBAR,
                //! Tree of Widget
                TREEVIEW,
                //! 2D Image
                IMAGE,
            };

            //! Widget placement (relative to the parent widget)
            enum AlignmentType {
                NONE,
                //! The child widget is centered and resized to the parent content size
                FILL,
                //! The child widget is centered (and take all the parent content size)
                CENTER,
                //! The child widget is horizontally centered
                HCENTER,
                //! The child widget is vertically centered
                VCENTER,
                //! The children are stack on the top
                TOP,
                //! The children are stack on the bottom
                BOTTOM,
                //! The children are stack on the left
                LEFT,
                //! The children are stack on the right
                RIGHT,
                //! The children are stack on the top and horizontally centered
                TOPCENTER,
                //! The children are stack on the bottom and horizontally centered
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
            explicit Widget(Type = WIDGET);

            ~Widget() override {}

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
            void setPos(float x,
                        float y);

            /** Returns the width of the widget, in pixels */
            [[nodiscard]] inline float getWidth() const { return rect.width; }

            /** Returns the height of the widget, in pixels */
            [[nodiscard]] inline float getHeight() const { return rect.height; }

            /** Resizes the widget */
            virtual void setSize(float width,
                                 float height);

            /** Returns the size & the position of the widget */
            [[nodiscard]] const Rect &getRect() const;

            /** Changes the size & position of the widget */
            void setRect(float x,
                         float y,
                         float width,
                         float height);

            /** Changes the size & position of the widget */
            void setRect(const Rect &);

            /** Returns the current widget placement */
            [[nodiscard]] AlignmentType getAlignment() const;

            /** Sets the widget placement. Calling this method involve
                redrawing the parent widget & resizing all the children widgets */
            void setAlignment(AlignmentType);

            /** Returns the current font of the widget */
            [[nodiscard]] Font& getFont();

            /** Sets the current font of the widget */
            void setFont(const shared_ptr<Font> &);

            /** Returns true if the widget have keyboard focus */
            [[nodiscard]] bool isFocused() const;

            /** Returns the parent widget, or nullptr */
            [[nodiscard]] shared_ptr<Widget> getParent() const;

            /** Adds a child widget.
                  Children widgets will be destroyed on parent destruction.
                    \param child	: child widget to add
                    \param alignment: placement
                    \param resource	: resource string
                    \param defaultPadding	: default padding
            */
            virtual shared_ptr<Widget> add(shared_ptr<Widget> child,
                                            AlignmentType       alignment,
                                            const string & resource = "",
                                            float          defaultPadding= 0);

            /** Removes a child widget */
            virtual void remove(shared_ptr<Widget> &);

            /** Removes all children widgets recursively */
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

            /** Sets to false make the widget background transparent */
            void setDrawBackground(bool drawBackground);

            [[nodiscard]] bool isPushed() const;

            [[nodiscard]] bool isPointed() const;

            [[nodiscard]] bool isFreezed() const;

            [[nodiscard]] bool isRedrawOnMouseEvent() const;

            [[nodiscard]] Rect getChildrenRect() const;

            void setFreezed(const bool f) { freeze = f; }

            void setPushed(const bool p) { pushed = p; }

            /** Force a refresh of the entire widget */
            void refresh() const;

            /** Changes widget resources. Use with caution ! */
            void setResource(shared_ptr<Resource>);

            /** Return the user defined group index */
            [[nodiscard]] uint32_t getGroupIndex() const;

            /** Set the user defined group index */
            void setGroupIndex(int32_t);

            /** Returns the user data */
            void *getUserData() const;

            /** set user data */
            void setUserData(void *);

            /** Return the transparency alpha value */
            [[nodiscard]] inline float getTransparency() const { return transparency; }

            /** Changes the transparency alpha value */
            void setTransparency(float alpha);

            void resizeChildren();

            void _setRedrawOnMouseEvent(const bool r) { redrawOnMouseEvent = r; }

            void _setMoveChildrenOnPush(const bool r) { moveChildrenOnPush = r; }

            [[nodiscard]] virtual list<shared_ptr<Widget>> &_getChildren() { return children; }

            void _draw(VectorRenderer &) const;
            Widget *setFocus(bool = true);

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

            virtual bool eventMouseDown(MouseButton,
                                        float,
                                        float);

            virtual bool eventMouseUp(MouseButton,
                                      float,
                                      float);

            virtual bool eventMouseMove(uint32_t,
                                        float,
                                        float);

            virtual void eventGotFocus();

            virtual void eventLostFocus();

            void *                    window{nullptr};
            void*                     style{nullptr};
            bool                      mouseMoveOnFocus{false};

        protected:
            Rect                      rect;
            Rect                      defaultRect;
            float                     hborder{0};
            float                     vborder{0};
            float                     padding{0};
            bool                      focused{false};
            bool                      allowFocus{false};
            bool                      allowChildren{true};
            bool                      drawBackground{true};
            bool                      moveChildrenOnPush{false};
            bool                      redrawOnMouseEvent{false};
            bool                      redrawOnMouseMove{false};
            float                     transparency{1.0f};
            Widget *                 parent{nullptr};
            Type                      type;
            AlignmentType             alignment{NONE};
            shared_ptr<Resource>     resource;
            list<shared_ptr<Widget>> children;

            void allowingFocus(bool = true);

            [[nodiscard]] inline virtual Rect _getDefaultRect() { return defaultRect; }

            virtual void _init(Widget &,
                               AlignmentType,
                               const string &,
                               float);

        private:
            bool             pushed{false};
            bool             pointed{false};
            bool             freeze{true};
            bool             enabled{true};
            bool             visible{true};
            shared_ptr<Font> font{nullptr};
            void *           userData{nullptr};
            int32_t          groupIndex{0};
            Rect             childrenRect;

            Widget *setNextFocus();


        };
    }
}
