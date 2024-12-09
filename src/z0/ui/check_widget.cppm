/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;

export module z0.ui.CheckWidget;

import z0.Constants;

import z0.ui.Widget;

export namespace z0 {
    namespace ui {
        /**
         * Super class for all two-states widgets
         */
        class CheckWidget : public Widget {
        public:
            //! State of the widget. Change on user action.
            enum State {
                //! Checked (aka ON)
                CHECK,
                //! Unchecked (aka OFF)
                UNCHECK
            };

            //! Return current state of the widget
            [[nodiscard]] inline State getState() const { return state; }

            //! Change the state of the widget
            virtual void setState(State S);

        protected:
            explicit CheckWidget(const Type T): Widget{T} { }

            bool eventMouseDown(MouseButton B, float X, float Y) override;

        private:
            State state{UNCHECK};
        };
    }

}
