/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.Event;

import z0.Constants;
import z0.Signal;

import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        /**
         * List of widgets events signals
         */
        struct Event {
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
            //! value of a ValueSelect widget changed
            static const string OnValueChange;
            //! value of a ValueSelect widget changed by the user
            //static const string OnValueUserChange;
            //! range of a ValueSelect widget changed
            static const string OnRangeChange;
            //! item list of a GList widget have changed
            //static const string OnInsertItem;
            //! item list of a GList widget have changed
            //static const string OnRemoveItem;
            //! a Window size changed
            static const string OnResize;
            //! a Window position changed
            static const string OnMove;

            Widget* source;
        };

        /**
         * Parameter for Event::OnClick
         */
        struct EventClick : Event {
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for Event::OnKeyDown and Event::OnKeyUp
         */
        struct EventKeyb : Event {
            //! Key code
            Key key;
            //! set this to true if the event have been consumed and will not be passed to widgets & nodes below
            bool consumed{false};
        };

        /**
         * Parameters for Event::OnMouseDown and Event::OnMouseUp
         */
        struct EventMouseButton : Event {
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
         * Parameters for Event::OnMouseMove
         */
        struct EventMouseMove : Event {
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
         * Parameters for Event::OnStateChange
         */
        struct EventState : Event {
            //! CheckWidget::State
            int32_t state;
        };

        /**
         * Parameters for Event::EventValue
         */
        struct EventValue : Event {
            float value;
            float previous;
        };

        /**
         * Parameters for Event::EventRange
         */
        struct EventRange : Event {
            float min;
            float max;
            float value;
        };

        struct EventTextChange : Event {
            const string text;
        };

        const Signal::signal Event::OnCreate{"on_create"};
        const Signal::signal Event::OnDestroy{"on_destroy"};
        const Signal::signal Event::OnKeyDown{"on_key_down"};
        const Signal::signal Event::OnKeyUp{"on_key_up"};
        const Signal::signal Event::OnMouseDown{"on_mouse_down"};
        const Signal::signal Event::OnMouseUp{"on_mouse_up"};
        const Signal::signal Event::OnMouseMove{"on_mouse_move"};
        const Signal::signal Event::OnGotFocus{"on_got_focus"};
        const Signal::signal Event::OnLostFocus{"on_lost_focus"};
        const Signal::signal Event::OnShow{"on_show"};
        const Signal::signal Event::OnHide{"on_hide"};
        const Signal::signal Event::OnEnable{"on_enable"};
        const Signal::signal Event::OnDisable{"on_disable"};
        const Signal::signal Event::OnTextChange{"on_text_change"};
        const Signal::signal Event::OnClick{"on_click"};
        const Signal::signal Event::OnStateChange{"on_state_change"};
        const Signal::signal Event::OnResize{"on_resize"};
        const Signal::signal Event::OnMove{"on_move"};
        const Signal::signal Event::OnValueChange{"on_value_change"};
        //const Signal::signal Event::OnValueUserChange{"on_value_use_change"};
        const Signal::signal Event::OnRangeChange{"on_range_change"};
        /*     const Signal::signal Event::OnInsertItem{"on_insert_item"};
            const Signal::signal Event::OnRemoveItem{"on_remove_item"};
            const Signal::signal Event::OnSelectItem{"on_select_item"};
         */
    }
}
