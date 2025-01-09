/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0.ui.ScrollBar;

import z0.Constants;
import z0.Signal;

import z0.ui.Box;
import z0.ui.Event;
import z0.ui.Rect;
import z0.ui.ValueSelect;

export namespace z0 {

    namespace ui {
        class ScrollBar : public ValueSelect {
        public:
            static constexpr auto LIFT_MINWIDTH{10};
            static constexpr auto LONGSTEP_MUX{5};

            enum Type {
                HORIZONTAL,
                VERTICAL
            };

            explicit ScrollBar(Type T = HORIZONTAL,
                               float min = 0,
                               float max = 100,
                               float value = 0,
                               float step = 1);

            inline Type getScrollBarType() const { return type; };

            void setResources(const string& RAREA, const string& RCAGE);

        private:
            Type type;
            bool onScroll{false};
            float scrollStart{0};
            shared_ptr<Box> liftArea;
            shared_ptr<Box> liftCage;

            bool eventMouseUp(MouseButton B, float X, float Y) override;

            bool eventMouseMove(uint32_t B, float X, float Y) override;

            void eventRangeChange() override;

            void eventValueChange(float prev) override;

            void onLiftAreaDown(const EventMouseButton* event) ;

            void onLiftCageDown(const EventMouseButton* event);

            void liftRefresh(const Rect& rect) const;
        };

        class VScrollBar : public ScrollBar {
        public:
            explicit VScrollBar(const float min = 0, const float max = 100, const float value = 0, const float step = 1):
                ScrollBar(VERTICAL, min, max, value, step) {
            }
        };

        class HScrollBar : public ScrollBar {
        public:
            explicit HScrollBar(const float min = 0, const float max = 100, const float value = 0, const float step = 1):
                ScrollBar(HORIZONTAL, min, max, value, step) {
            }
        };

    }
}
