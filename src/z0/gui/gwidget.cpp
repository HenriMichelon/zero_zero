#include "z0/gui/gwindow.h"
#include "z0/gui/gwidget.h"

#include <algorithm>

namespace z0 {

//----------------------------------------------------------
    GWidget::GWidget(Type T): focused(false),
                                    allowFocus(false), allowChilds(true),
                                    drawBackground(true),
                                    moveChildsOnPush(false),
                                    redrawOnMouseEvent(false),
                                    redrawOnMouseMove(false),
                                    mouseMoveOnFocus(false),
                                    font(nullptr),
                                    parent(nullptr), window(nullptr),
            //popup(nullptr),
                                    type(T),
                                    alignment(NONE),
                                    layout(nullptr)
    {
        transparent = (type == WIDGET);
        mHborder = mVborder = mPadding = mGroupIndex = 0;
        mUserData = nullptr;
        moveChildsNow = mPushed = mPointed = false;
        mVisible = mFreeze = mEnabled = true;
    }


//----------------------------------------------------------
    bool GWidget::isVisible() const
    {
        const shared_ptr<GWidget>p = this;
        do {
            if (!p->mVisible) { return false; }
        }
        while ( (p = p->parent.get()) != nullptr);
        return (window && window->isVisible());
    }


//----------------------------------------------------------
    void GWidget::show(bool S)
    {
        if (mVisible == S) return;
        mVisible = S;
        if (mVisible) {
            eventShow();
        }
        else {
            eventHide();
        }
    }


//----------------------------------------------------------
    void GWidget::enable(bool S)
    {
        if (mEnabled == S) return;
        mEnabled = S;
        if (mEnabled) {
            eventEnable();
        }
        else {
            eventDisable();
        }
    }


//----------------------------------------------------------
    void GWidget::setPos(int32_t L, int32_t T, bool REDRAW)
    {
        if ((L == rect.left) && (T == rect.top)) return;
        eventMove(L, T, REDRAW);
    }


//----------------------------------------------------------
    void GWidget::setSize(uint32_t W, uint32_t H, bool REDRAW)
    {
        if ((W == rect.width) && (H == rect.height)) return;
        if (parent) { parent->refresh(rect); }
        rect.width = W;
        rect.height = H;
        eventResize(REDRAW);
    }


    //----------------------------------------------------------
    shared_ptr<GWidget> GWidget::setNextFocus()
    {
        if (focused) {
            setFocus(false);
        }
        else {
            shared_ptr<GWidget> r = setFocus();
            if (r) return r;
        }

        if (!parent) return nullptr;
        uint32_t idx;
        shared_ptr<GWidget> p = parent;
        GWidget* s = this;

        ListIterator<GWidget> list;
        do {
            list = p->childs;
            if (!((idx = list.IndexOf(*s)) == p->childs.Count())) { break; }
            s = p;
            p = p->parent;
            if (!p) return s->SetFocus();
        } while (true);
        return list[idx+1].SetNextFocus();
    }


    //----------------------------------------------------------
    GWidget* GWidget::setFocus(bool F)
    {
        if (!mEnabled) return nullptr;

        if (F && (!allowFocus)) {
            for (auto& child : childs) {
                auto w = child.setFocus(F);
                if (w) return w;
            }
            return nullptr;
        }

        if (focused != F) {
            focused = F;
            if (F) {
                if (!mFreeze) { refresh(); }
                call(GEvent::OnGotFocus);
            }
            else {
                call(GEvent::OnLostFocus);
                /*shared_ptr<GWidget>p = parent;
                while (p && (!p->DrawBackground())) p = p->parent;
                if (p) { p->Refresh(rect); }*/
            }
        }
        return this;
    }


//----------------------------------------------------------
    void GWidget::allowingFocus(bool A)
    {
        allowFocus = A;
        for (auto& child : childs) {
            child.allowingFocus(false);
        }
    }


//----------------------------------------------------------
    void GWidget::init(GWidget&WND, AlignmentType ALIGN,
                       const string&RES, uint32_t P)
    {
        WND.mPadding = P;
        WND.alignment = ALIGN;
        if (!WND.font) { WND.font = font; }
        WND.window = window;
        WND.layout = layout;
        WND.parent = this;
        layout->addResource(WND, RES);
        WND.eventCreate();
        if (window->isVisible() && resource) { resizeChildren(); }
        WND.mFreeze = false;
    }


//----------------------------------------------------------
    void GWidget::remove(GWidget&W)
    {
        ListIterator<GWidget> list(childs);
        uint32_t idx = list.IndexOf(W);
        if (idx) {
            /*dprintf("drop %x, %x, %x\n", this, &W, parent);*/
            W.parent = nullptr;
            list = W.Childs();
            while (!list.End()) {
                W.remove(list.Next());
            }
            childs.remove(idx);
            resizeChildren();
        }
        refresh();
    }


//----------------------------------------------------------
    void GWidget::removeAll()
    {
        for (auto& child: childs) {
            child.removeAll();
        }
        childs.clear();
        refresh();
    }


//----------------------------------------------------------
    GWidget& GWidget::add(GWidget&WND, AlignmentType ALIGN, const string&RES,
                          uint32_t P)
    {
        //PRE(window, "GWidget::Add: widget must be added to another widget before use");
        if (!allowChilds) return WND;
        childs.push_back(WND);
        init(WND, ALIGN, RES, P);
        return WND;
    }


//----------------------------------------------------------
    shared_ptr<GWidget> GWidget::add(shared_ptr<GWidget>WND, AlignmentType ALIGN, const string&RES,
                          uint32_t P)
    {
        //PRE(window, "GWidget::Add: widget must be added to another widget before use");
        if (!allowChilds) return WND;
        childs.push_back(*WND);
        init(*WND, ALIGN, RES, P);
        return WND;
    }


//----------------------------------------------------------
    void GWidget::connect(GEvent::Type TYP, void* OBJ,
                          const GEventFunction FUNC)
    {
        //PRE(OBJ, "Invalid object for event slot connection");
        //PRE(FUNC, "Invalid method for event slot connection");
        slots[TYP].obj = OBJ;
        slots[TYP].func = FUNC;
    }


//----------------------------------------------------------
    void GWidget::simulate(GEvent::Type TYP, GEvent *EVT)
    {
        switch (TYP) {
            case GEvent::OnClick:
                eventMouseDown(MouseButton::MOUSE_BUTTON_LEFT,getRect().left, getRect().top);
                eventMouseUp(MouseButton::MOUSE_BUTTON_LEFT, getRect().left, getRect().top);
                break;
            default:
                call(TYP, EVT);
                break;
        }
    }


//----------------------------------------------------------
    void GWidget::call(GEvent::Type TYP, GEvent *EVT)
    {
        const GWidget::GEventSlot &slot = slots[TYP];
        if (slot.func) {
            //ASSERT(slot.obj);
            (slot.obj->*slot.func)(*this, EVT);
        }
    }


//----------------------------------------------------------
    void GWidget::eventCreate()
    {
        call(GEvent::OnCreate);
    }


//----------------------------------------------------------
    void GWidget::eventDestroy()
    {
        for (auto& child: childs) {
            child.eventDestroy();
        }
        //if (popup) popup->EventDestroy();
        call(GEvent::OnDestroy);
    }


//----------------------------------------------------------
    void GWidget::eventShow()
    {
        if (mVisible) {
            call(GEvent::OnShow);
            for (auto& child: childs) {
                child.eventShow();
            }
            if (parent) { refresh(); }
        }
    }


//----------------------------------------------------------
    void GWidget::eventHide()
    {
        if (!mVisible) {
            for (auto& child: childs) {
                child.eventHide();
            }
            if (parent) { parent->refresh(rect); }
            call(GEvent::OnHide);
        }
    }


//----------------------------------------------------------
    void GWidget::eventEnable()
    {
        call(GEvent::OnEnable);
        for (auto& child: childs) {
            child.enable();
        }
        refresh();
    }


//----------------------------------------------------------
    void GWidget::eventDisable()
    {
        for (auto& child: childs) {
            child.enable(false);
        }
        call(GEvent::OnDisable);
        refresh();
    }


//----------------------------------------------------------
    void GWidget::eventMove(int32_t L, int32_t T, bool REDRAW)
    {
        GRect old_rect(rect);
        int32_t diffX = rect.left - L;
        int32_t diffY = rect.top - T;
        rect.left = L;
        rect.top = T;
        for (auto& w: childs) {
            w.setPos(w.rect.left - diffX, w.rect.top - diffY, false);
        }
        GEventPos event(T, L);
        call(GEvent::OnMove, &event);
        if (parent) { parent->refresh(old_rect); }
        if (REDRAW) { refresh(); }
    }


//----------------------------------------------------------
    void GWidget::eventResize(bool REDRAW)
    {
        if (mFreeze) return;
        mFreeze = true;
        if (REDRAW) { refresh(); }
        if (parent && (!parent->mFreeze)) parent->resizeChildren();
        if (rect.width && rect.height) {
            GEventSize event(rect.width, rect.height);
            call(GEvent::OnResize, &event);
            resizeChildren();
        }
        mFreeze = false;
    }


//----------------------------------------------------------
    void GWidget::resizeChildren()
    {
        if (!layout) { return; }
        mFreeze = true;
        layout->resize(*this, rect, *resource);
        GRect arect = rect;
        arect.left += mHborder + mPadding + mChildsRect.left;
        if (arect.width > (2*mHborder + 2*mPadding) ) {
            arect.width -= 2*mHborder + 2*mPadding;
        }
        else {
            arect.width = 0;
        }
        arect.top += mVborder + mPadding + mChildsRect.top;
        if (arect.height > (2*mVborder + 2*mPadding) ) {
            arect.height -= 2*mVborder + 2*mPadding;
        }
        else {
            arect.height = 0;
        }

        ListIterator<GWidget> list(childs);
        while ((arect.width>0) && (arect.height>0) && (!list.End())) {
            GWidget &w = list.Next();
            GRect wrect = w.rect;
            /*dprintf("ResizeChild: %d,%d,%d,%d / %d,%d,%d,%d\n",
                    wrect.left, wrect.top, wrect.width, wrect.height,
                    arect.left, arect.top, arect.width, arect.height);*/
            switch (w.alignment)
            {
                case CLIENT:
                    wrect = arect;
                    arect.width = 0;
                    arect.height = 0;
                    break;
                case CENTER:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + (arect.width - wrect.width)/2;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + (arect.height- wrect.height)/2;
                    arect.width = 0;
                    arect.height = 0;
                    break;
                case VCENTER:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + (arect.width - wrect.width)/2;
                    wrect.top = arect.top;
                    wrect.height = arect.height;
                    arect.width = 0;
                    break;
                case HCENTER:
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + (arect.height- wrect.height)/2;
                    wrect.left = arect.left;
                    wrect.width = arect.width;
                    arect.width = 0;
                    break;
                case TOP:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    wrect.width = arect.width;
                    arect.top += wrect.height + mPadding;
                    //arect.top = min(rect.height, arect.top + wrect.height + mPadding);
                    arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case LEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    wrect.height = arect.height;
                    arect.left += wrect.width + mPadding;
                    //arect.left = min(rect.width, arect.left + wrect.width + mPadding);
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case BOTTOM:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    wrect.width = arect.width;
                    break;
                case RIGHT:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    wrect.top = arect.top;
                    wrect.height = arect.height;
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case TOPCENTER:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + (arect.width - wrect.width)/2;
                    arect.top += wrect.height + mPadding;
                    //arect.top = min(rect.height, arect.top + wrect.height + mPadding);
                    arect.height -= wrect.height + 2*mPadding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case BOTTOMCENTER:
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + (arect.width - wrect.width)/2;
                    arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case LEFTCENTER:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + (arect.height- wrect.height)/2;
                    arect.left += wrect.width + mPadding;
                    //arect.left = min(rect.width, arect.left + wrect.width + mPadding);
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case RIGHTCENTER:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + (arect.height- wrect.height)/2;
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case TOPLEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.top += wrect.height + mPadding;
                    //arect.top = min(rect.height, arect.top + wrect.height + mPadding);
                    arect.height -= wrect.height + 2*mPadding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case BOTTOMLEFT:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case BOTTOMRIGHT:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    break;
                case TOPRIGHT:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    arect.height -= wrect.height + 2*mPadding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*mPadding)));
                    arect.top += wrect.height + mPadding;
                    //arect.top = min(rect.height, arect.top + wrect.height + mPadding);
                    break;
                case LEFTTOP:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.left += wrect.width + mPadding;
                    //arect.left = min(rect.width, arect.left + wrect.width + mPadding);
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case LEFTBOTTOM:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.left += wrect.width + mPadding;
                    //arect.left = min(rect.width, arect.left + wrect.width + mPadding);
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case RIGHTBOTTOM:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case RIGHTTOP:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    arect.width = max(0l, int32_t(arect.width - (wrect.width + 2*mPadding)));
                    break;
                case CORNERTOPLEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.top += wrect.height + mPadding;
                    break;
                case CORNERBOTTOMLEFT:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    break;
                case CORNERBOTTOMRIGHT:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    break;
                case CORNERTOPRIGHT:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    arect.top += wrect.height + mPadding;
                    break;
                default:
                    continue;
                    break;
            }
            /*dprintf("ResizeChild: %d,%d,%d,%d / %d,%d,%d,%d\n",
                    wrect.left, wrect.top, wrect.width, wrect.height,
                    arect.left, arect.top, arect.width, arect.height);*/
            w.rect = wrect;
            w.eventResize(false);
        }
        mFreeze = false;
    }


//----------------------------------------------------------
    Key GWidget::EventKeybDown(Key K)
    {
        //PRE(mEnabled, "Disabled widget received a KeybUp event");
        GEventKeyb event(K);
        call(GEvent::OnKeybDown, &event);
        return event.key;
    }


//----------------------------------------------------------
    Key GWidget::eventKeybUp(Key K)
    {
        //PRE(mEnabled, "Disabled widget received a KeybUp event");
        if (focused) {
            GEventKeyb event(K);
            call(GEvent::OnKeybUp, &event);
            return event.key;
        }
        return K;
    }


//----------------------------------------------------------
/*void GWidget::ClosePopup()
{
	if (popup) popup->Show(false);
	ListIterator<GWidget> list(childs);
	while (!list.End())  { list.Next().ClosePopup(); }
}*/


//----------------------------------------------------------
    shared_ptr<GWidget> GWidget::EventMouseDown(MouseButton B, int32_t X, int32_t Y)
    {
        if (!mEnabled) return nullptr;

        /*uint32_t close = popup && popup->Visible();
        ClosePopup();*/

        mPushed = !isTransparent();
        shared_ptr<GWidget>wfocus = nullptr;

        for (auto& w: childs) {
            if (w.getRect().contains(X, Y)) {
                wfocus = w.eventMouseDown(B, X, Y);
                if ((!wfocus) && w.allowFocus) { wfocus = w.setFocus(); }
                if (w.redrawOnMouseEvent && w.getRect().contains(X, Y) && (!w.isTransparent())) {
                    w.refresh();
                }
            }
        }
        if (redrawOnMouseEvent && rect.contains(X, Y) && (!transparent)) { refresh(); }

        GEventMouse event(B, X, Y);
        call(GEvent::OnMouseDown, &event);
        return wfocus;
    }


//----------------------------------------------------------
    void GWidget::eventMouseUp(MouseButton B, int32_t X, int32_t Y)
    {
        if (!mEnabled) { return; }
        bool r = redrawOnMouseEvent && (rect.contains(X, Y) || mPushed);
        mPushed = false;
        for (auto& w : childs) {
            w.eventMouseUp(B, X, Y);
            if (w.redrawOnMouseEvent && (w.getRect().Contains(X, Y) || w.mPushed) && (!w.isTransparent())) {
                w.refresh();
            }
        }
        if (r && (!transparent)) { refresh(); }
        GEventMouse event(B, X, Y);
        call(GEvent::OnMouseUp, &event);
    }


//----------------------------------------------------------
    void GWidget::eventMouseMove(MouseButton B, int32_t X, int32_t Y)
    {
        if (!mEnabled) { return; }
        bool p = rect.contains(X, Y);
        for (auto& w : childs) {
            p = w.getRect().contains(X, Y);
            if (w.redrawOnMouseMove && (w.mPointed != p)) {
                w.mPointed = p;
                w.refresh();
            }
            if (p) {
                w.eventMouseMove(B, X, Y);
            } else if (w.mPushed) {
                w.eventMouseUp(B, X, Y);
            }
        }
        if (redrawOnMouseMove && (mPointed != p) && (!transparent)) { refresh(); }
        GEventMouse event(B, X, Y);
        call(GEvent::OnMove, &event);
    }



//----------------------------------------------------------
    void GWidget::eventGotFocus()
    {
        call(GEvent::OnGotFocus);
    }


//----------------------------------------------------------
    void GWidget::eventLostFocus()
    {
        call(GEvent::OnLostFocus);
    }


//----------------------------------------------------------
    bool GWidget::clipRect(GRect&R, const GRect& R1, const GRect& R2) const
    {
        if ((!R1.width) || (!R1.height) ||
            (!R2.width) || (!R2.height)) {
            return false;
        }
        if (R1.left > R2.left) {
            R.width = min(R1.width, R2.width - (R1.left - R2.left));
            R.left = R1.left;
        }
        else {
            R.width = min(R2.width, R1.width - (R2.left - R1.left));
            R.left = R2.left;
        }
        if (R1.top > R2.top) {
            R.height = min(R1.height, R2.height - (R1.top - R2.top));
            R.top = R1.top;
        }
        else {
            R.height = min(R2.height, R1.height - (R2.top - R1.top));
            R.top = R2.top;
        }
        return ((R.width > 0) && (R.height > 0));
    }


//----------------------------------------------------------
    void GWidget::maxRect(GRect&R, const GRect A, const GRect B) const
    {
        if ((!A.width) || (!A.height)) {
            R = B;
        }
        else if ((!B.width) || (!B.height)) {
            R = A;
        }
        else {
            R.width = max(0, max(A.left + int32_t(A.width), B.left + int32_t(B.width)));
            R.height = max(0, max(A.top + int32_t(A.height), B.top + int32_t(B.height)));
            R.left = max(0, min(A.left, B.left));
            R.top = max(0, min(A.top, B.top));
            R.width -= R.left;
            R.height -= R.top;
        }
        /*dprintf("MaxRect: %d,%d,%d,%d / %d,%d,%d,%d = %d,%d,%d,%d\n",
            A.left, A.top, A.width, A.height,
            B.left, B.top, B.width, B.height,
            R.left, R.top, R.width, R.height);*/
    }


//----------------------------------------------------------
    void GWidget::setPadding(int32_t P)
    {
        mPadding = P;
        eventResize(true);
    }


//----------------------------------------------------------
    void GWidget::setDrawBackground(bool D)
    {
        drawBackground = D;
        refresh();
    }


//----------------------------------------------------------
    void GWidget::setAlignment(GWidget::AlignmentType ALIGN)
    {
        alignment = ALIGN;
        eventResize(true);
    }


//----------------------------------------------------------
    void GWidget::refresh()
    {
        if (!mFreeze) {
            refresh(rect);
        }
    }


//----------------------------------------------------------
    void GWidget::refresh(const GRect&R, bool PARENT)
    {
        if ((!mVisible) || (window && (!window->isVisible())) ||
            (!R.width) || (!R.height) || (!rect.width) || (!rect.height)) {
            return;
        }
        if (PARENT && (transparent || (!mVisible)) && parent) {
            parent->refresh(R);
        }
        GRect refreshzone;
        if (clipRect(refreshzone, rect, R)) {
            shared_ptr<GWidget>p = parent;
            while (p) {
                if (!clipRect(refreshzone, refreshzone, p->rect)) {
                    //Debug(dprintf("EventDraw(%x): refreshzone out of parent widget rect\n", this));
                    return;
                }
                p = p->parent;
            }
            maxRect(mRefreshRect, mRefreshRect, refreshzone);
            /*Debug(dprintf("Refresh(%x, %d): %d,%d,%d,%d -> %d,%d,%d,%d\n", this, Type(),
                    R.left, R.top, R.width, R.height,
                    mRefreshRect.left, mRefreshRect.top, mRefreshRect.width, mRefreshRect.height	);)*/
        }
    }


//----------------------------------------------------------
    void GWidget::flushRefresh(GRect&R)
    {
        if (mRefreshRect.width && mRefreshRect.height) {
            maxRect(R, R, mRefreshRect);
            reallyDraw(mRefreshRect);
        }
        for (auto& w : childs) {
            w.flushRefresh(R);
        }
    }


//----------------------------------------------------------
    void GWidget::reallyDraw(const GRect &R)
    {
        GRect refreshzone;
        if (!clipRect(refreshzone, rect, R)) {
            //Debug(dprintf("ReallyDraw(%x): refreshzone out of widget rect\n", this));
            mRefreshRect.top =
            mRefreshRect.left =
            mRefreshRect.width =
            mRefreshRect.height = 0;
            return;
        }
        shared_ptr<GWidget>p = parent;
        while (p) {
            if (!p->rect.contains(refreshzone)) {
                //Debug(dprintf("ReallyDraw(%x): refreshzone out of parent widget rect\n", this));
                mRefreshRect.top =
                mRefreshRect.left =
                mRefreshRect.width =
                mRefreshRect.height = 0;
                return;
            }
            p = p->parent;
        }

        maxRect(refreshzone, mRefreshRect, refreshzone);
        /*Debug(dprintf("ReallyDraw(%x): %d,%d,%d,%d -> %d,%d,%d,%d\n", this,
                R.left, R.top, R.width, R.height,
                refreshzone.left, refreshzone.top, refreshzone.width, refreshzone.height	);)*/

        eventDraw(refreshzone, true);
        GRect inner_refreshzone(rect.left + mHborder,
                                rect.top + mVborder,
                                rect.width - mHborder * 2,
                                rect.height - mVborder * 2);
        if (clipRect(inner_refreshzone, inner_refreshzone, refreshzone)) {
            GRect child_refreshzone;
            for (auto& child : childs) {
                if ((mPushed && moveChildsOnPush) || (moveChildsNow)) {
                    child.moveChildsNow = true;
                    child.mFreeze = true;
                    ++child.rect.left;
                    ++child.rect.top;
                    if (clipRect(child_refreshzone, child.rect, inner_refreshzone)) {
                        if (rect.contains(child_refreshzone)) {
                            child.reallyDraw(child_refreshzone);
                        }
                    }
                    child.mFreeze = false;
                    --child.rect.left;
                    --child.rect.top;
                }
                else if (clipRect(child_refreshzone, child.rect, inner_refreshzone)) {
                    child.moveChildsNow = false;
                    if (rect.contains(child_refreshzone)) {
                        child.reallyDraw(child_refreshzone);
                    }
                }
            }
        }
        eventDraw(refreshzone, false);
        mRefreshRect.top =
        mRefreshRect.left =
        mRefreshRect.width =
        mRefreshRect.height = 0;
    }


//----------------------------------------------------------
    void GWidget::eventDraw(const GRect&R, bool BEFORECHILDS)
    {
        if (isVisible()) {
            //display->SetClip(R); // XXX
            layout->draw(*this, *resource, R, BEFORECHILDS);
            if (BEFORECHILDS) { call(GEvent::OnDraw); }
        }
    }


//------------------------------------------------------------
    void GWidget::setFont(shared_ptr<Font>F)
    {
        font = &F;
        resizeChildren();
        refresh();
    }


//------------------------------------------------------------
    void GWidget::setGroupIndex(int32_t IDX)
    {
        mGroupIndex = IDX;
    }

//------------------------------------------------------------
    void GWidget::setData(void* DATA)
    {
        mUserData = DATA;
    }


//------------------------------------------------------------
    bool GWidget::getDrawBackground() const {
        return drawBackground;
    }

//------------------------------------------------------------
    GRect& GWidget::getChildsRect() {
        return mChildsRect;
    }


//------------------------------------------------------------
    bool& GWidget::isPointed() {
        return mPointed;
    }


//------------------------------------------------------------
    GWidget::AlignmentType GWidget::getAlignment() const {
        return alignment;
    }


//------------------------------------------------------------
    GWidget::WidgetType GWidget::getType() const {
        return type;
    }


//------------------------------------------------------------
    shared_ptr<GWidget> GWidget::getParent() const {
        return parent;
    }


//------------------------------------------------------------
    bool GWidget::isEnabled() const {
        return mEnabled;
    }


//------------------------------------------------------------
    bool GWidget::isFocused() const {
        return focused;
    }


//------------------------------------------------------------
    const GRect& GWidget::getRect() const {
        return rect;
    }


//------------------------------------------------------------
    void GWidget::setRect(int32_t L, int32_t T, uint32_t W, uint32_t H, bool R)
    {
        setPos(L, T, R);
        setSize(W, H, R);
    }


//------------------------------------------------------------
    void GWidget::setRect(const GRect&R, bool RE)
    {
        setRect(R.left, R.top, R.width, R.height, RE);
    }


//------------------------------------------------------------
    bool& GWidget::isPushed() {
        return mPushed;
    }


//------------------------------------------------------------
    bool& GWidget::isFreezed() {
        return mFreeze;
    }


//------------------------------------------------------------
    bool& GWidget::isMoveChildsOnPush() {
        return moveChildsOnPush;
    }


//------------------------------------------------------------
    bool& GWidget::isTransparent() {
        return transparent;
    }

//------------------------------------------------------------
    bool& GWidget::redrawOnMouseEvent() {
        return redrawOnMouseEvent;
    }


//------------------------------------------------------------
    uint32_t GWidget::getPadding() const {
        return mPadding;
    }


//------------------------------------------------------------
    uint32_t GWidget::getVBorder() const {
        return mVborder;
    }


//------------------------------------------------------------
    uint32_t GWidget::getHBorder() const {
        return mHborder;
    }


//-----------------------------------------------------------
    void GWidget::setVBorder(uint32_t B) {
        mVborder = B;
        if (!mFreeze) { resizeChildren(); }
        refresh();
    }


//-----------------------------------------------------------
    void GWidget::setHBorder(uint32_t B) {
        mHborder = B;
        if (!mFreeze) { resizeChildren(); }
        refresh();
    }


//------------------------------------------------------------
    uint32_t GWidget::getGroupIndex() const {
        return mGroupIndex;
    }


//------------------------------------------------------------
    void* GWidget::getData() const {
        return mUserData;
    }



}