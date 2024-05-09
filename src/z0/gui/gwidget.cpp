#include "z0/gui/gwindow.h"
#include "z0/gui/gwidget.h"

#include <algorithm>

namespace z0 {

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
        hborder = vborder = padding = groupIndex = 0;
        userData = nullptr;
        moveChildsNow = pushed = pointed = false;
        visible = freeze = enabled = true;
    }


    bool GWidget::isVisible() const {
        const GWidget* p = this;
        do {
            if (!p->visible) { return false; }
        }
        while ( (p = p->parent) != nullptr);
        return (window && window->isVisible());
    }

    void GWidget::show(bool S) {
        if (visible == S) return;
        visible = S;
        if (visible) {
            eventShow();
        }
        else {
            eventHide();
        }
    }

    void GWidget::enable(bool S) {
        if (enabled == S) return;
        enabled = S;
        if (enabled) {
            eventEnable();
        }
        else {
            eventDisable();
        }
    }

    void GWidget::setPos(int32_t L, int32_t T, bool REDRAW) {
        if ((L == rect.left) && (T == rect.top)) return;
        eventMove(L, T, REDRAW);
    }

    void GWidget::setSize(uint32_t W, uint32_t H, bool REDRAW) {
        if ((W == rect.width) && (H == rect.height)) return;
        if (parent) { parent->refresh(rect); }
        rect.width = W;
        rect.height = H;
        eventResize(REDRAW);
    }

    GWidget* GWidget::setNextFocus() {
        if (focused) {
            setFocus(false);
        }
        else {
            GWidget* r = setFocus();
            if (r) return r;
        }

        if (!parent) return nullptr;
        uint32_t idx;
        GWidget* p = parent;
        GWidget* s = this;

       /* auto it = children.begin();
        do {
            list = p->children;
            if (!((idx = list.IndexOf(*s)) == p->children.Count())) { break; }
            s = p;
            p = p->parent;
            if (!p) return s->SetFocus();
        } while (true);
        return list[idx+1].SetNextFocus();*/
       die("Not implemented");
       return nullptr;
    }

    GWidget* GWidget::setFocus(bool F) {
        if (!enabled) return nullptr;

        if (F && (!allowFocus)) {
            for (auto& child : children) {
                auto w = child->setFocus(F);
                if (w) return w;
            }
            return nullptr;
        }

        if (focused != F) {
            focused = F;
            if (F) {
                if (!freeze) { refresh(); }
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

    void GWidget::allowingFocus(bool A) {
        allowFocus = A;
        for (auto& child : children) {
            child->allowingFocus(false);
        }
    }

    void GWidget::init(GWidget&WND, AlignmentType ALIGN,
                       const string&RES, uint32_t P) {
        WND.padding = P;
        WND.alignment = ALIGN;
        if (!WND.font) { WND.font = font; }
        WND.window = window;
        WND.layout = layout;
        WND.parent = this;
        layout->addResource(WND, RES);
        WND.eventCreate();
        if (window->isVisible() && resource) { resizeChildren(); }
        WND.freeze = false;
    }

    void GWidget::remove(shared_ptr<GWidget>& W) {
        auto it = std::find(children.begin(), children.end(), W);
        if (it != children.end()) {
            W->parent = nullptr;
            for (auto& child: W->getChildren()) {
                W->remove(child);
            }
            children.remove(W);
            resizeChildren();
        }
        refresh();
    }

    void GWidget::removeAll() {
        for (auto& child: children) {
            child->removeAll();
        }
        children.clear();
        refresh();
    }

    shared_ptr<GWidget> GWidget::add(shared_ptr<GWidget>WND, AlignmentType ALIGN, const string&RES,
                          uint32_t P) {
        //PRE(window, "GWidget::Add: widget must be added to another widget before use");
        if (!allowChilds) return WND;
        children.push_back(WND);
        init(*WND, ALIGN, RES, P);
        return WND;
    }

    void GWidget::connect(GEvent::Type TYP, void* OBJ,
                          const GEventFunction FUNC) {
        //PRE(OBJ, "Invalid object for event slot connection");
        //PRE(FUNC, "Invalid method for event slot connection");
        slots[TYP].obj = static_cast<GWidget *>(OBJ); // TODO cast to Object ??
        slots[TYP].func = FUNC;
    }

    void GWidget::simulate(GEvent::Type TYP,  shared_ptr<GEvent> EVT) {
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

    void GWidget::call(GEvent::Type TYP,  shared_ptr<GEvent> EVT) {
        const GWidget::GEventSlot &slot = slots[TYP];
        if (slot.func) {
            //ASSERT(slot.obj);
            //(slot.obj->*slot.func)(*this, EVT);
            die("Not implemented");
        }
    }

    void GWidget::eventCreate() {
        call(GEvent::OnCreate);
    }

    void GWidget::eventDestroy() {
        for (auto& child: children) {
            child->eventDestroy();
        }
        //if (popup) popup->EventDestroy();
        call(GEvent::OnDestroy);
    }

    void GWidget::eventShow() {
        if (visible) {
            call(GEvent::OnShow);
            for (auto& child: children) {
                child->eventShow();
            }
            if (parent) { refresh(); }
        }
    }

    void GWidget::eventHide() {
        if (!visible) {
            for (auto& child: children) {
                child->eventHide();
            }
            if (parent) { parent->refresh(rect); }
            call(GEvent::OnHide);
        }
    }

    void GWidget::eventEnable() {
        call(GEvent::OnEnable);
        for (auto& child: children) {
            child->enable();
        }
        refresh();
    }

    void GWidget::eventDisable() {
        for (auto& child: children) {
            child->enable(false);
        }
        call(GEvent::OnDisable);
        refresh();
    }

    void GWidget::eventMove(int32_t L, int32_t T, bool REDRAW) {
        GRect old_rect(rect);
        int32_t diffX = rect.left - L;
        int32_t diffY = rect.top - T;
        rect.left = L;
        rect.top = T;
        for (auto& w: children) {
            w->setPos(w->rect.left - diffX, w->rect.top - diffY, false);
        }
        auto event = make_shared<GEventPos>(T, L);
        call(GEvent::OnMove, event);
        if (parent) { parent->refresh(old_rect); }
        if (REDRAW) { refresh(); }
    }

    void GWidget::eventResize(bool REDRAW) {
        if (freeze) return;
        freeze = true;
        if (REDRAW) { refresh(); }
        if (parent && (!parent->freeze)) parent->resizeChildren();
        if (rect.width && rect.height) {
            auto event = make_shared<GEventSize>(rect.width, rect.height);
            call(GEvent::OnResize, event);
            resizeChildren();
        }
        freeze = false;
    }

    void GWidget::resizeChildren() {
        if (!layout) { return; }
        freeze = true;
        layout->resize(*this, *resource);
        GRect arect = rect;
        arect.left += hborder + padding + childrenRect.left;
        if (arect.width > (2 * hborder + 2 * padding) ) {
            arect.width -= 2 * hborder + 2 * padding;
        }
        else {
            arect.width = 0;
        }
        arect.top += vborder + padding + childrenRect.top;
        if (arect.height > (2 * vborder + 2 * padding) ) {
            arect.height -= 2 * vborder + 2 * padding;
        }
        else {
            arect.height = 0;
        }

        auto it = children.begin();
        while ((arect.width>0) && (arect.height>0) && (it != children.end())) {
            auto& w = *it;
            GRect wrect = w->rect;
            switch (w->alignment)
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
                    arect.top += wrect.height + padding;
                    //arect.top = min(rect.height, arect.top + wrect.height + padding);
                    arect.height = std::max(0, int32_t(arect.height - (wrect.height + 2 * padding)));
                    break;
                case LEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    wrect.height = arect.height;
                    arect.left += wrect.width + padding;
                    //arect.left = min(rect.width, arect.left + wrect.width + padding);
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case BOTTOM:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.height = std::max(0, int32_t(arect.height - (wrect.height + 2 * padding)));
                    wrect.width = arect.width;
                    break;
                case RIGHT:
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    wrect.top = arect.top;
                    wrect.height = arect.height;
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case TOPCENTER:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + (arect.width - wrect.width)/2;
                    arect.top += wrect.height + padding;
                    //arect.top = min(rect.height, arect.top + wrect.height + padding);
                    arect.height -= wrect.height + 2 * padding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*padding)));
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
                    arect.height = std::max(0, int32_t(arect.height - (wrect.height + 2 * padding)));
                    break;
                case LEFTCENTER:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + (arect.height- wrect.height)/2;
                    arect.left += wrect.width + padding;
                    //arect.left = min(rect.width, arect.left + wrect.width + padding);
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
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
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case TOPLEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.top += wrect.height + padding;
                    //arect.top = min(rect.height, arect.top + wrect.height + padding);
                    arect.height -= wrect.height + 2 * padding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*padding)));
                    break;
                case BOTTOMLEFT:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.height = std::max(0, int32_t(arect.height - (wrect.height + 2 * padding)));
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
                    arect.height = std::max(0, int32_t(arect.height - (wrect.height + 2 * padding)));
                    break;
                case TOPRIGHT:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    arect.height -= wrect.height + 2 * padding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*padding)));
                    arect.top += wrect.height + padding;
                    //arect.top = min(rect.height, arect.top + wrect.height + padding);
                    break;
                case LEFTTOP:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.left += wrect.width + padding;
                    //arect.left = min(rect.width, arect.left + wrect.width + padding);
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case LEFTBOTTOM:
                    wrect.left = arect.left;
                    if (wrect.height > arect.height)
                        wrect.top = 0;
                    else
                        wrect.top = arect.top + arect.height - wrect.height;
                    arect.left += wrect.width + padding;
                    //arect.left = min(rect.width, arect.left + wrect.width + padding);
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
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
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case RIGHTTOP:
                    wrect.top = arect.top;
                    if (wrect.width > arect.width)
                        wrect.left = 0;
                    else
                        wrect.left = arect.left + arect.width - wrect.width;
                    arect.width = std::max(0, int32_t(arect.width - (wrect.width + 2 * padding)));
                    break;
                case CORNERTOPLEFT:
                    wrect.left = arect.left;
                    wrect.top = arect.top;
                    arect.top += wrect.height + padding;
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
                    arect.top += wrect.height + padding;
                    break;
                default:
                    continue;
                    break;
            }
            w->rect = wrect;
            w->eventResize(false);
        }
        freeze = false;
    }

    Key GWidget::eventKeybDown(Key K) {
        //PRE(enabled, "Disabled widget received a KeybUp event");
        auto event = make_shared<GEventKeyb>(K);
        call(GEvent::OnKeybDown, event);
        return event->key;
    }

    Key GWidget::eventKeybUp(Key K) {
        //PRE(enabled, "Disabled widget received a KeybUp event");
        if (focused) {
            auto event = make_shared<GEventKeyb>(K);
            call(GEvent::OnKeybUp, event);
            return event->key;
        }
        return K;
    }

    shared_ptr<GWidget> GWidget::eventMouseDown(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) return nullptr;

        /*uint32_t close = popup && popup->Visible();
        ClosePopup();*/

        pushed = !isTransparent();
        GWidget* wfocus = nullptr;

        for (auto& w: children) {
            if (w->getRect().contains(X, Y)) {
                wfocus = w->eventMouseDown(B, X, Y).get();
                if ((!wfocus) && w->allowFocus) { wfocus = w->setFocus(); }
                if (w->redrawOnMouseEvent && w->getRect().contains(X, Y) && (!w->isTransparent())) {
                    w->refresh();
                }
            }
        }
        if (redrawOnMouseEvent && rect.contains(X, Y) && (!transparent)) { refresh(); }

        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseDown, event);
        return shared_ptr<GWidget>(wfocus);
    }

    void GWidget::eventMouseUp(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) { return; }
        bool r = redrawOnMouseEvent && (rect.contains(X, Y) || pushed);
        pushed = false;
        for (auto& w : children) {
            w->eventMouseUp(B, X, Y);
            if (w->redrawOnMouseEvent && (w->getRect().contains(X, Y) || w->pushed) && (!w->isTransparent())) {
                w->refresh();
            }
        }
        if (r && (!transparent)) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseUp, event);
    }

    void GWidget::eventMouseMove(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) { return; }
        bool p = rect.contains(X, Y);
        for (auto& w : children) {
            p = w->getRect().contains(X, Y);
            if (w->redrawOnMouseMove && (w->pointed != p)) {
                w->pointed = p;
                w->refresh();
            }
            if (p) {
                w->eventMouseMove(B, X, Y);
            } else if (w->pushed) {
                w->eventMouseUp(B, X, Y);
            }
        }
        if (redrawOnMouseMove && (pointed != p) && (!transparent)) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMove, event);
    }

    void GWidget::eventGotFocus() {
        call(GEvent::OnGotFocus);
    }

    void GWidget::eventLostFocus() {
        call(GEvent::OnLostFocus);
    }

    bool GWidget::clipRect(GRect&R, const GRect& R1, const GRect& R2) const {
        if ((!R1.width) || (!R1.height) ||
            (!R2.width) || (!R2.height)) {
            return false;
        }
        if (R1.left > R2.left) {
            R.width = std::min(R1.width, R2.width - (R1.left - R2.left));
            R.left = R1.left;
        }
        else {
            R.width = std::min(R2.width, R1.width - (R2.left - R1.left));
            R.left = R2.left;
        }
        if (R1.top > R2.top) {
            R.height = std::min(R1.height, R2.height - (R1.top - R2.top));
            R.top = R1.top;
        }
        else {
            R.height = std::min(R2.height, R1.height - (R2.top - R1.top));
            R.top = R2.top;
        }
        return ((R.width > 0) && (R.height > 0));
    }

    void GWidget::maxRect(GRect&R, const GRect A, const GRect B) const {
        if ((!A.width) || (!A.height)) {
            R = B;
        }
        else if ((!B.width) || (!B.height)) {
            R = A;
        }
        else {
            R.width = std::max(0, std::max(A.left + int32_t(A.width), B.left + int32_t(B.width)));
            R.height = std::max(0, std::max(A.top + int32_t(A.height), B.top + int32_t(B.height)));
            R.left = std::max(0, std::min(A.left, B.left));
            R.top = std::max(0, std::min(A.top, B.top));
            R.width -= R.left;
            R.height -= R.top;
        }
    }

    void GWidget::setPadding(int32_t P) {
        padding = P;
        eventResize(true);
    }

    void GWidget::setDrawBackground(bool D) {
        drawBackground = D;
        refresh();
    }

    void GWidget::setAlignment(GWidget::AlignmentType ALIGN) {
        alignment = ALIGN;
        eventResize(true);
    }

    void GWidget::refresh() {
        if (!freeze) {
            refresh(rect);
        }
    }
     void GWidget::refresh(const GRect&R, bool PARENT)
    {
        if ((!visible) || (window && (!window->isVisible())) ||
            (!R.width) || (!R.height) || (!rect.width) || (!rect.height)) {
            return;
        }
        if (PARENT && (transparent || (!visible)) && parent) {
            parent->refresh(R);
        }
    }


    void GWidget::flushRefresh() {
        eventDraw(false);
        for (auto& w : children) {
            w->flushRefresh();
        }
    }

    void GWidget::eventDraw(bool BEFORECHILDS) {
        if (isVisible()) {
            layout->draw(*this, *resource, BEFORECHILDS);
            if (BEFORECHILDS) { call(GEvent::OnDraw); }
        }
    }

    void GWidget::setFont(shared_ptr<Font>&F) {
        font = F;
        resizeChildren();
        refresh();
    }

    void GWidget::setGroupIndex(int32_t IDX) {
        groupIndex = IDX;
    }
    void GWidget::setData(void* DATA) {
        userData = DATA;
    }

    bool GWidget::getDrawBackground() const {
        return drawBackground;
    }


    GRect& GWidget::getChildrenRect() {
        return childrenRect;
    }

    bool& GWidget::isPointed() {
        return pointed;
    }

    GWidget::AlignmentType GWidget::getAlignment() const {
        return alignment;
    }

    GWidget::Type GWidget::getType() const {
        return type;
    }

    shared_ptr<GWidget> GWidget::getParent() const {
        return shared_ptr<GWidget>(parent);
    }

    bool GWidget::isEnabled() const {
        return enabled;
    }

    bool GWidget::isFocused() const {
        return focused;
    }

    const GRect& GWidget::getRect() const {
        return rect;
    }

    void GWidget::setRect(int32_t L, int32_t T, uint32_t W, uint32_t H, bool R) {
        setPos(L, T, R);
        setSize(W, H, R);
    }

    void GWidget::setRect(const GRect&R, bool RE) {
        setRect(R.left, R.top, R.width, R.height, RE);
    }

    bool& GWidget::isPushed() {
        return pushed;
    }

    bool& GWidget::isFreezed() {
        return freeze;
    }

    bool& GWidget::isMoveChildsOnPush() {
        return moveChildsOnPush;
    }

    bool& GWidget::isTransparent() {
        return transparent;
    }

    bool& GWidget::isRedrawOnMouseEvent() {
        return redrawOnMouseEvent;
    }

    int32_t GWidget::getPadding() const {
        return padding;
    }

    uint32_t GWidget::getVBorder() const {
        return vborder;
    }

    uint32_t GWidget::getHBorder() const {
        return hborder;
    }

    void GWidget::setVBorder(uint32_t B) {
        vborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    void GWidget::setHBorder(uint32_t B) {
        hborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    uint32_t GWidget::getGroupIndex() const {
        return groupIndex;
    }

    void* GWidget::getData() const {
        return userData;
    }

}