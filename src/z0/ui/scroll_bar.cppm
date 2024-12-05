/*
 * Copyright (c) 2024 Henri Michelon
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
#define LIFT_MINWIDTH 	10
#define LONGSTEP_MUX	5

    namespace ui {
        class ScrollBar : public ValueSelect {
        public:
            enum Type {
                HORIZONTAL,
                VERTICAL
            };

            explicit ScrollBar(const Type T = HORIZONTAL,
                                const float min = 0,
                                const float max = 100,
                                const float value = 0,
                                const float step = 1):
                ValueSelect{SCROLLBAR, min, max, value, step},
                type{T} {
            }

            Type getScrollBarType() const { return type; };

            void setResources(const string& RAREA, const string& RCAGE) {
                if (liftArea == nullptr) {
                    liftArea = make_shared<Box>();
                    liftCage = make_shared<Box>();
                    mouseMoveOnFocus = true;
                    add(liftArea, FILL, RAREA);
                    add(liftCage, NONE, RCAGE);
                    liftArea->connect(Event::OnMouseDown,
                        [this](auto p) { this->onLiftAreaDown(static_cast<const EventMouseButton *>(p)); });
                    liftCage->connect(Event::OnMouseDown,
                        [this](auto p) { this->onLiftCageDown(static_cast<const EventMouseButton *>(p)); });
                    liftCage->_setRedrawOnMouseEvent(true);
                    liftCage->_setMoveChildrenOnPush(true);
                }
            }

        private:
            Type type;
            bool onScroll{false};
            float scrollStart{0};
            shared_ptr<Box> liftArea;
            shared_ptr<Box> liftCage;

            bool eventMouseUp(const MouseButton B, const float X, const float Y) override {
                onScroll = false;
                return ValueSelect::eventMouseUp(B, X, Y);
            }

            bool eventMouseMove(const uint32_t B, const float X, const float Y) override {
                if (onScroll) {
                    if (getRect().contains(X, Y)) {
                        float diff;
                        float size;
                        const float nbvalues = max - min;
                        if (type == VERTICAL) {
                            diff = Y - liftArea->getRect().y;
                            size = liftArea->getHeight() - liftCage->getHeight();
                        }
                        else {
                            diff = X - liftArea->getRect().x;
                            size = liftArea->getWidth() - liftCage->getWidth();
                        }
                        if (diff > scrollStart) {
                            float newval = ((diff - scrollStart) * nbvalues) / size;
                            if (type == VERTICAL) {
                                newval = max - newval;
                            }
                            const float prev = value;
                            value = std::min(std::max(newval, min), max);
                            eventValueChange(prev);
                        }
                    }
                    else {
                        onScroll = false;
                    }
                }
                ValueSelect::eventMouseMove(B, X, Y);
                return true;
            }

            void eventRangeChange() override {
                if (liftArea == nullptr) { return; }
                liftCage->setPushed(onScroll);
                const Rect& rect = liftArea->getRect();
                if (rect.width && rect.height && ((max - min) > 0.0f)) {
                    liftRefresh(rect);
                    ValueSelect::eventRangeChange();
                }
            }

            void eventValueChange(const float prev) override {
                liftCage->setPushed(onScroll);
                const Rect& rect = liftArea->getRect();
                if (rect.width && rect.height && ((max - min) > 0.0f)) {
                    liftRefresh(rect);
                    ValueSelect::eventValueChange(prev);
                }
            }

            void onLiftAreaDown(const EventMouseButton* event) {
                if (liftCage->getRect().contains(event->x, event->y)) { return; }
                const float longStep = step * LONGSTEP_MUX;
                float diff = 0;
                if (type == VERTICAL) {
                    if (event->y < liftCage->getRect().y)
                        diff = longStep;
                    else if (event->y > (liftCage->getRect().y + liftCage->getHeight()))
                        diff = -longStep;
                    else
                        return;
                }
                else {
                    if (event->x < liftCage->getRect().x)
                        diff = -longStep;
                    else if (event->x > (liftCage->getRect().x + liftCage->getWidth()))
                        diff = longStep;
                    else
                        return;
                }
                const float prev = value;
                value = std::min(std::max(value + diff, min), max);
                eventRangeChange();
                ValueSelect::eventValueChange(prev);
            }

            void onLiftCageDown(const EventMouseButton* event) {
                onScroll = true;
                if (type == VERTICAL) {
                    scrollStart = event->y - liftCage->getRect().y;
                }
                else {
                    scrollStart = event->x - liftCage->getRect().x;
                }
            }

            void liftRefresh(const Rect& rect) const {
                float size;
                if (type == VERTICAL) {
                    size = rect.height;
                }
                else {
                    size = rect.width;
                }
                float liftSize = LIFT_MINWIDTH;
                const float nbvalues = max - min;
                if (size >= nbvalues) {
                    liftSize = size - nbvalues;
                }
                if (type == VERTICAL) {
                    liftCage->setSize(rect.width, liftSize);
                    liftSize = liftCage->getHeight();
                }
                else {
                    liftCage->setSize(liftSize, rect.height);
                    liftSize = liftCage->getWidth();
                }
                const auto liftPos = ((value - min) * (size - liftSize)) / nbvalues;
                if (type == VERTICAL) {
                    //log(to_string(liftPos), to_string(liftSize), to_string(size));
                    liftCage->setPos(rect.x, rect.y + size - liftSize - liftPos);
                }
                else {
                    liftCage->setPos(rect.x + liftPos, rect.y);
                }
                liftArea->refresh();
                liftCage->refresh();
            }
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
