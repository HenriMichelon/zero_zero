module;
#include "z0/libraries.h"

export module Z0:GScrollBar;

import :Constants;
import :Rect;
import :Signal;
import :GEvent;
import :GValueSelect;
import :GBox;

export namespace z0 {
#define LIFT_MINWIDTH 	10
#define LONGSTEP_MUX	5

    class GScrollBar : public GValueSelect {
    public:
        enum Type {
            HORIZONTAL,
            VERTICAL
        };

        GScrollBar(Type T = HORIZONTAL,
                   float min = 0,
                   float max = 100,
                   float value = 0,
                   float step = 1):
            GValueSelect{SCROLLBAR, min, max, value, step},
            type{T} {
        }

        Type getScrollBarType() const { return type; };

        void setResources(const string& RAREA, const string& RCAGE) {
            if (liftArea == nullptr) {
                liftArea = make_shared<GBox>();
                liftCage = make_shared<GBox>();
                mouseMoveOnFocus = true;
                add(liftArea, FILL, RAREA);
                add(liftCage, NONE, RCAGE);
                liftArea->connect(GEvent::OnMouseDown, this,
                                  Signal::Handler(&GScrollBar::onLiftAreaDown));
                liftCage->connect(GEvent::OnMouseDown, this,
                                  Signal::Handler(&GScrollBar::onLiftCageDown));
                liftCage->_setRedrawOnMouseEvent(true);
                liftCage->_setMoveChildrenOnPush(true);
            }
        }

    private:
        Type type;
        bool onScroll{false};
        float scrollStart{0};
        shared_ptr<GBox> liftArea;
        shared_ptr<GBox> liftCage;

        bool eventMouseUp(MouseButton B, float X, float Y) override {
            onScroll = false;
            return GValueSelect::eventMouseUp(B, X, Y);
        }

        bool eventMouseMove(uint32_t B, float X, float Y) override {
            if (onScroll) {
                if (getRect().contains(X, Y)) {
                    float diff;
                    float size;
                    float nbvalues = max - min;
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
                        float prev = value;
                        value = std::min(std::max(newval, min), max);
                        eventValueChange(prev);
                    }
                }
                else {
                    onScroll = false;
                }
            }
            GValueSelect::eventMouseMove(B, X, Y);
            return true;
        }

        void eventRangeChange() override {
            if (liftArea == nullptr) { return; }
            liftCage->setPushed(onScroll);
            const Rect& rect = liftArea->getRect();
            if (rect.width && rect.height && ((max - min) > 0.0f)) {
                liftRefresh(rect);
                GValueSelect::eventRangeChange();
            }
        }

        void eventValueChange(float prev) override {
            liftCage->setPushed(onScroll);
            const Rect& rect = liftArea->getRect();
            if (rect.width && rect.height && ((max - min) > 0.0f)) {
                liftRefresh(rect);
                GValueSelect::eventValueChange(prev);
            }
        }

        void onLiftAreaDown(GEventMouseButton* event) {
            if (liftCage->getRect().contains(event->x, event->y)) { return; }
            float longStep = step * LONGSTEP_MUX;
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
            float prev = value;
            value = std::min(std::max(value + diff, min), max);
            eventRangeChange();
            GValueSelect::eventValueChange(prev);
        }

        void onLiftCageDown(GEventMouseButton* event) {
            onScroll = true;
            if (type == VERTICAL) {
                scrollStart = event->y - liftCage->getRect().y;
            }
            else {
                scrollStart = event->x - liftCage->getRect().x;
            }
        }

        void liftRefresh(const Rect& rect) const {
            float size, liftSize, liftPos;
            float nbvalues = max - min;
            if (type == VERTICAL) {
                size = rect.height;
            }
            else {
                size = rect.width;
            }
            if (size >= nbvalues) {
                liftSize = size - nbvalues;
            }
            else {
                liftSize = LIFT_MINWIDTH;
            }
            if (type == VERTICAL) {
                liftCage->setSize(rect.width, liftSize);
                liftSize = liftCage->getHeight();
            }
            else {
                liftCage->setSize(liftSize, rect.height);
                liftSize = liftCage->getWidth();
            }
            liftPos = ((value - min) * (size - liftSize)) / nbvalues;
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

    class GVScrollBar : public GScrollBar {
    public:
        GVScrollBar(uint32_t min = 0, uint32_t max = 100, uint32_t value = 0, uint32_t step = 1):
            GScrollBar(VERTICAL, min, max, value, step) {
        };
    };

    class GHScrollBar : public GScrollBar {
    public:
        GHScrollBar(uint32_t min = 0, uint32_t max = 100, uint32_t value = 0, uint32_t step = 1):
            GScrollBar(HORIZONTAL, min, max, value, step) {
        };
    };
}