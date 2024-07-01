#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbutton.h"
#include "z0/gui/gbox.h"
#include "z0/gui/gpanel.h"
#include "z0/gui/gvalue_select.h"
#include "z0/gui/gscroll_bar.h"
#include "z0/application.h"
#endif

namespace z0 {

    #define LIFT_MINWIDTH 	10
    #define LONGSTEP_MUX	5

    GScrollBar::GScrollBar(Type T, uint32_t MIN, uint32_t MAX,
                        uint32_t VAL, uint32_t STEP):
        GValueSelect(SCROLLBAR), type(T), onScroll(FALSE)  {
        min = MIN;
        max = MAX;
        value = VAL;
        step = STEP;
        mouseMoveOnFocus = TRUE;
        liftArea = make_shared<GBox>();
        liftCage = make_shared<GBox>();
    }

    void GScrollBar::setResources(const string&RAREA, const string&RCAGE) {
        add(liftArea, FILL, RAREA);
        add(liftCage, NONE, RCAGE);
        liftArea->connect(GEvent::OnMouseDown, this,
                        Signal::Handler(&GScrollBar::onLiftAreaDown));
        liftCage->connect(GEvent::OnMouseDown, this,
                        Signal::Handler(&GScrollBar::onLiftCageDown));
        liftCage->_setRedrawOnMouseEvent(true);
        liftCage->_setMoveChildrenOnPush(true);
    }

    void GScrollBar::eventRangeChange() {
        liftCage->setPushed(onScroll);
        const Rect &rect = liftArea->getRect();
        if (rect.width && rect.height && ((max - min) != 0)) {
            uint32_t size, liftSize, liftPos;
            uint32_t nbvalues = max - min;
            if (type == VERTICAL)
                size = rect.height;
            else
                size = rect.width;
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
            liftPos = uint32_t((static_cast<float>(value - min) * (size - liftSize)) / nbvalues);
            if (type == VERTICAL) {
                liftCage->setPos(rect.x, rect.y + liftPos);
            }
            else {
                liftCage->setPos(rect.x + liftPos, rect.y);
            }
            liftArea->refresh();
            liftCage->refresh();
            GValueSelect::eventResize();
            GValueSelect::eventRangeChange() ;
        }
    }

    void GScrollBar::eventValueChange(float prev) {
        liftCage->setPushed(onScroll);
        const Rect &rect = liftArea->getRect();
        if (rect.width && rect.height && ((max - min) != 0)) {
            uint32_t size, liftSize, liftPos;
            uint32_t nbvalues = max - min;
            if (type == VERTICAL)
                size = rect.height;
            else
                size = rect.width;
            if (type == VERTICAL) {
                liftSize = liftCage->getHeight();
            }
            else {
                liftSize = liftCage->getWidth();
            }
            liftPos = uint32_t((static_cast<float>(value - min) * (size - liftSize)) / nbvalues);
            if (type == VERTICAL) {
                liftCage->setPos(rect.x, rect.y + liftPos);
            }
            else {
                liftCage->setPos(rect.x + liftPos, rect.y);
            }
            liftCage->refresh();
            liftArea->refresh();
            GValueSelect::eventValueChange(prev);
        }
    }

    bool GScrollBar::eventMouseMove(MouseButton B, float X, float Y) {
        if (onScroll) {
            if (getRect().contains(X, Y)) {
                float diff;
                uint32_t size;
                uint32_t nbvalues = max - min;
                if (type == VERTICAL) {
                    diff = Y - liftArea->getRect().y;
                    size = liftArea->getHeight() - liftCage->getHeight();
                }
                else {
                    diff = X - liftArea->getRect().x;
                    size = liftArea->getWidth() - liftCage->getWidth();
                }
                if (diff > float(scrollStart)) {
                    float newval = float(((diff - scrollStart) * nbvalues) / size);
                    float prev = value;
                    value = std::min(std::max(newval, min), max);
                    eventValueChange(prev);
                }
            }
            else {
                onScroll = FALSE;
            }
        }
        GValueSelect::eventMouseMove(B, X, Y);
        return true;
    }

    bool GScrollBar::eventMouseUp(MouseButton B, float X, float Y) {
        onScroll = FALSE;
        return GValueSelect::eventMouseUp(B, X, Y);
    }

    void GScrollBar::onLiftCageDown(GEventMouseButton*event) {
        onScroll = TRUE;
        if (type == VERTICAL) {
            scrollStart = event->y - liftCage->getRect().y;
        }
        else {
            scrollStart = event->x - liftCage->getRect().x;
        }
    }

    void GScrollBar::onLiftAreaDown(GEventMouseButton*event) {
        if (liftCage->getRect().contains(event->x, event->y)) { return; }
        float longStep = step * LONGSTEP_MUX;
        float diff = 0;
        if (type == VERTICAL)
        {
            if (event->y < (uint32_t)liftCage->getRect().y)
                diff = -longStep;
            else if (event->y > (liftCage->getRect().y + liftCage->getHeight()))
                diff = longStep;
            else
                return;
        }
        else
        {
            if (event->x < (uint32_t)liftCage->getRect().x)
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

}