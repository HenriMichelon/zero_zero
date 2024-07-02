#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gbox.h"
#include "z0/gui/gvalue_select.h"
#include "z0/gui/gscroll_bar.h"
#include "z0/application.h"
#endif

namespace z0 {

    #define LIFT_MINWIDTH 	10
    #define LONGSTEP_MUX	5

    GScrollBar::GScrollBar(Type T, 
                           float MIN, 
                           float MAX,
                           float VAL, 
                           float STEP):
        GValueSelect{SCROLLBAR, MIN, MAX, VAL, STEP},
        type{T}  {
    }

    void GScrollBar::setResources(const string&RAREA, const string&RCAGE) {
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

    void GScrollBar::liftRefresh(const Rect &rect) {
        float size, liftSize, liftPos;
        float nbvalues = max - min;
        if (type == VERTICAL) {
            size = rect.height;
        } else {
            size = rect.width;
        }
        if (size >= nbvalues) {
            liftSize = size - nbvalues;
        } else {
            liftSize = LIFT_MINWIDTH;
        }
        if (type == VERTICAL) {
            liftCage->setSize(rect.width, liftSize);
            liftSize = liftCage->getHeight();
        } else {
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
    
    void GScrollBar::eventRangeChange() {
        if (liftArea == nullptr) { return; }
        liftCage->setPushed(onScroll);
        const Rect &rect = liftArea->getRect();
        if (rect.width && rect.height && ((max - min) > 0.0f)) {
            liftRefresh(rect);
            GValueSelect::eventRangeChange() ;
        }
    }

    void GScrollBar::eventValueChange(float prev) {
        liftCage->setPushed(onScroll);
        const Rect &rect = liftArea->getRect();
        if (rect.width && rect.height && ((max - min) > 0.0f)) {
            liftRefresh(rect);
            GValueSelect::eventValueChange(prev);
        }
    }

    bool GScrollBar::eventMouseMove(uint32_t B, float X, float Y) {
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

    bool GScrollBar::eventMouseUp(MouseButton B, float X, float Y) {
        onScroll = false;
        return GValueSelect::eventMouseUp(B, X, Y);
    }

    void GScrollBar::onLiftCageDown(GEventMouseButton*event) {
        onScroll = true;
        if (type == VERTICAL) {
            scrollStart = event->y -liftCage->getRect().y ;
        }
        else {
            scrollStart = event->x - liftCage->getRect().x;
        }
    }

    void GScrollBar::onLiftAreaDown(GEventMouseButton*event) {
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
        } else {
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

}