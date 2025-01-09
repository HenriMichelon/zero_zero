/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>

module z0.ui.ValueSelect;

import z0.ui.Event;

namespace z0 {
    namespace ui {

        ValueSelect::ValueSelect(const Type T,
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

        void ValueSelect::setMin(const float V) {
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

        void ValueSelect::setMax(const float V) {
            if (max == V) return;
            if (V < min) return;
            max = V;
            if (value > max) setValue(max);
            resizeChildren();
            eventRangeChange();
            auto event = EventRange{.min = min, .max = max, .value = value};
            emit(Event::OnRangeChange, &event);
        }

        void ValueSelect::setValue(const float V) {
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

        void ValueSelect::setStep(const float V) {
            assert(V != 0 && "ValueSelect: can't use a step of 0");
            if (step == V) return;
            step = V;
            eventRangeChange();
            refresh();
        }

        void ValueSelect::eventResize() {
            Widget::eventResize();
            eventRangeChange();
        }

        void ValueSelect::eventRangeChange() {
            auto event = EventRange{.min = min, .max = max, .value = value};
            emit(Event::OnRangeChange, &event);
        }

        void ValueSelect::eventValueChange(const float prev) {
            auto event = EventValue{.value = value, .previous = prev};
            emit(Event::OnValueChange, &event);
        }

    }
}
