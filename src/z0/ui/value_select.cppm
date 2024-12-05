/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>

export module z0.ui.ValueSelect;

import z0.ui.Event;
import z0.ui.Widget;

export namespace z0 {

    namespace ui {
        class ValueSelect : public Widget {
        public:
            ValueSelect(const Type T,
                         const float MIN,
                         const float MAX,
                         const float VAL,
                         const float STEP):
                Widget{T},
                min{MIN},
                max{MAX},
                value{VAL},
                step{STEP} {
            }

            ~ValueSelect() override = default;

            float getMin() const { return min; }
            float getMax() const { return max; }
            float getValue() const { return value; }
            float getStep() const { return step; }

            virtual void setMin(const float V) {
                if (min == V) return;
                if (min > max) return;
                min = V;
                if (value < min) setValue(min);
                resizeChildren();
                eventRangeChange();
                refresh();
                auto event = EventRange{.min = min, .max = max, .value = value};
                emit(Event::OnRangeChange, &event);
            }

            virtual void setMax(const float V) {
                if (max == V) return;
                if (V < min) return;
                max = V;
                if (value > max) setValue(max);
                resizeChildren();
                eventRangeChange();
                auto event = EventRange{.min = min, .max = max, .value = value};
                emit(Event::OnRangeChange, &event);
            }

            virtual void setValue(const float V) {
                if (value == V) { return; }
                const float prev = value;
                value = V;
                if (V < min) {
                    value = min;
                }
                if (V > max) {
                    value = max;
                }
                eventRangeChange();
                refresh();
                if (parent) parent->refresh();
                auto event = EventValue{.value = value, .previous = prev};
                emit(Event::OnValueChange, &event);
            }

            virtual void setStep(const float V) {
                assert(V != 0 && "ValueSelect: can't use a step of 0");
                if (step == V) return;
                step = V;
                eventRangeChange();
                refresh();
            }

        protected:
            float min;
            float max;
            float value;
            float step;

            void eventResize() override {
                Widget::eventResize();
                eventRangeChange();
            }

            virtual void eventRangeChange() {
                auto event = EventRange{.min = min, .max = max, .value = value};
                emit(Event::OnRangeChange, &event);
            }

            virtual void eventValueChange(const float prev) {
                auto event = EventValue{.value = value, .previous = prev};
                emit(Event::OnValueChange, &event);
            }
        };
    }
}
