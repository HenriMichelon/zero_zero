/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.GEvent;

import z0.Constants;
import z0.Signal;

import z0.GWidget;

export namespace z0 {

    namespace ui {
        /**
         * List of widgets events signals
         */
        struct GEvent : public Signal::Parameters {
            //! called after widget creation (all widgets)
            static const string OnCreate;
            //! called before widget destruction (all widgets)
            static const string OnDestroy;
            //! called when the user press a key & the widget have the keyboard focus (all widgets)
            static const string OnKeyDown;
            //! called when the user press a key & the widget have the keyboard focus (all widgets)
            static const string OnKeyUp;
            //! the mouse button have been pressed above the widget or a child (all widgets)
            static const string OnMouseDown;
            //! the mouse button have been pressed above the widget or a child (all widgets)
            static const string OnMouseUp;
            //! the mouse have been moved above the widget (all widgets)
            static const string OnMouseMove;
            //! widget acquire keyboard focus (all widgets)
            static const string OnGotFocus;
            //! widget lost keyboard focus (all widgets)
            static const string OnLostFocus;
            //! called after visibility change (all widgets)
            static const string OnShow;
            //! called before visibility change (all widgets)
            static const string OnHide;
            //! called after state change (all widgets)
            static const string OnEnable;
            //! called after state change (all widgets)
            static const string OnDisable;
            //! text content of the widget have changed
            static const string OnTextChange;
            //! called when the user click on the widget (buttons)
            static const string OnClick;
            //! a CheckWidget state changed
            static const string OnStateChange;
            //! value of a GValueSelect widget changed
            static const string OnValueChange;
            //! value of a GValueSelect widget changed by the user
            //static const string OnValueUserChange;
            //! range of a GValueSelect widget changed
            static const string OnRangeChange;
            //! item list of a GList widget have changed
            //static const string OnInsertItem;
            //! item list of a GList widget have changed
            //static const string OnRemoveItem;
            //! a GWindow size changed
            static const string OnResize;
            //! a GWindow position changed
            static const string OnMove;

            GWidget* source;
        };

        /**
         * Parameter for GEvent::OnClick
         */
        struct GEventClick : GEvent {
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for GEvent::OnKeyDown and GEvent::OnKeyUp
         */
        struct GEventKeyb : GEvent {
            //! Key code
            Key key;
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for GEvent::OnMouseDown and GEvent::OnMouseUp
         */
        struct GEventMouseButton : GEvent {
            //! Mouse button
            MouseButton button;
            //! X coord
            float x;
            //! Y coord
            float y;
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for GEvent::OnMouseMove
         */
        struct GEventMouseMove : GEvent {
            //! Mouse button states
            uint32_t buttonsState;
            //! X coord
            float x;
            //! Y coord
            float y;
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for GEvent::OnStateChange
         */
        struct GEventState : GEvent {
            //! GCheckWidget::State
            int32_t state;
        };

        /**
         * Parameters for GEvent::GEventValue
         */
        struct GEventValue : GEvent {
            float value;
            float previous;
        };

        /**
         * Parameters for GEvent::GEventRange
         */
        struct GEventRange : GEvent {
            float min;
            float max;
            float value;
        };

        struct GEventTextChange : GEvent {
            const string text;
        };

        const Signal::signal GEvent::OnCreate{"on_create"};
        const Signal::signal GEvent::OnDestroy{"on_destroy"};
        const Signal::signal GEvent::OnKeyDown{"on_key_down"};
        const Signal::signal GEvent::OnKeyUp{"on_key_up"};
        const Signal::signal GEvent::OnMouseDown{"on_mouse_down"};
        const Signal::signal GEvent::OnMouseUp{"on_mouse_up"};
        const Signal::signal GEvent::OnMouseMove{"on_mouse_move"};
        const Signal::signal GEvent::OnGotFocus{"on_got_focus"};
        const Signal::signal GEvent::OnLostFocus{"on_lost_focus"};
        const Signal::signal GEvent::OnShow{"on_show"};
        const Signal::signal GEvent::OnHide{"on_hide"};
        const Signal::signal GEvent::OnEnable{"on_enable"};
        const Signal::signal GEvent::OnDisable{"on_disable"};
        const Signal::signal GEvent::OnTextChange{"on_text_change"};
        const Signal::signal GEvent::OnClick{"on_click"};
        const Signal::signal GEvent::OnStateChange{"on_state_change"};
        const Signal::signal GEvent::OnResize{"on_resize"};
        const Signal::signal GEvent::OnMove{"on_move"};
        const Signal::signal GEvent::OnValueChange{"on_value_change"};
        //const Signal::signal GEvent::OnValueUserChange{"on_value_use_change"};
        const Signal::signal GEvent::OnRangeChange{"on_range_change"};
        /*     const Signal::signal GEvent::OnInsertItem{"on_insert_item"};
            const Signal::signal GEvent::OnRemoveItem{"on_remove_item"};
            const Signal::signal GEvent::OnSelectItem{"on_select_item"};
         */
    }
}
