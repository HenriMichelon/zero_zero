module;
#include <cassert>

export module Z0:GValueSelect;

import :GWidget;
import :GEvent;

export namespace z0 {
    class GValueSelect : public GWidget {
    public:
        GValueSelect(const Type T,
                     const float MIN,
                     const float MAX,
                     const float VAL,
                     const float STEP):
            GWidget{T},
            min{MIN},
            max{MAX},
            value{VAL},
            step{STEP} {
        }

        ~GValueSelect() override = default;

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
            auto event = GEventRange{.min = min, .max = max, .value = value};
            emit(GEvent::OnRangeChange, &event);
        }

        virtual void setMax(const float V) {
            if (max == V) return;
            if (V < min) return;
            max = V;
            if (value > max) setValue(max);
            resizeChildren();
            eventRangeChange();
            auto event = GEventRange{.min = min, .max = max, .value = value};
            emit(GEvent::OnRangeChange, &event);
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
            auto event = GEventValue{.value = value, .previous = prev};
            emit(GEvent::OnValueChange, &event);
        }

        virtual void setStep(const float V) {
            assert(V != 0 && "GValueSelect: can't use a step of 0");
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
            GWidget::eventResize();
            eventRangeChange();
        }

        virtual void eventRangeChange() {
            auto event = GEventRange{.min = min, .max = max, .value = value};
            emit(GEvent::OnRangeChange, &event);
        }

        virtual void eventValueChange(const float prev) {
            auto event = GEventValue{.value = value, .previous = prev};
            emit(GEvent::OnValueChange, &event);
        }
    };
}
