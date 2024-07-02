#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gvalue_select.h"
#include "z0/application.h"
#endif

namespace z0 {
        
    GValueSelect::GValueSelect(Type T,
                               float MIN, 
                               float MAX,
                               float VAL, 
                               float STEP): 
        GWidget{T},
        min{MIN},
        max{MAX},
        value{VAL},
        step{STEP} {
    }


    void GValueSelect::setValue(float V) {
        if (value == V) { return; }
        float prev = value;
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
        auto event = GEventValue{ .value = value, .previous = prev };
        emit(GEvent::OnValueChange, &event);
    }

    void GValueSelect::setMin(float V) {
        if (min == V) return;
        if (min > max) return;
        min = V;
        if (value < min) setValue(min);
        resizeChildren();
        eventRangeChange();
        refresh();
        auto event = GEventRange{ .min = min, .max = max, .value = value };
        emit(GEvent::OnRangeChange, &event);
    }

    void GValueSelect::setMax(float V) {
        if (max == V) return;
        if (V < min) return;
        max = V;
        if (value > max) setValue(max);
        resizeChildren();
        eventRangeChange();
        auto event = GEventRange{ .min = min, .max = max, .value = value };
        emit(GEvent::OnRangeChange, &event);
    }

    void GValueSelect::setStep(float V) {
        assert(V != 0 && "GValueSelect: can't use a step of 0");
        if (step == V) return;
        step = V;
        eventRangeChange();
        refresh();
    }

    void GValueSelect::eventValueChange(float prev) { 
         auto event = GEventValue{ .value = value, .previous = prev };
        emit(GEvent::OnValueChange, &event);
    }

    void GValueSelect::eventRangeChange() {
        auto event = GEventRange{ .min = min, .max = max, .value = value };
        emit(GEvent::OnRangeChange, &event);
    }

    float GValueSelect::getValue() const { return value; }

    float GValueSelect::getMin() const { return min; }

    float GValueSelect::getMax() const { return max; }

    float GValueSelect::getStep() const { return step; }

    void GValueSelect::eventResize() {
        GWidget::eventResize();
        eventRangeChange();
    }
}