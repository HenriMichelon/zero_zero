#include "z0/gui/gwindow.h"
#include "z0/gui/gwidget.h"

#include <algorithm>
#include <utility>

namespace z0 {

    GWidget::GWidget(Type T):
        type{T},
        transparent{T == WIDGET} {
    }

    void GWidget::draw(VectorRenderer &R) const {
        if (!isVisible()) return;
        layout->draw(*this, *resource, R, true);
        for(auto& child: children) {
            child->draw(R);
        }
        layout->draw(*this, *resource, R, false);
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

    void GWidget::setPos(int32_t x, int32_t y) {
        if ((x == rect.x) && (y == rect.y)) return;
        eventMove(x, y);
    }

    void GWidget::setSize(int32_t W, int32_t H) {
        if ((W == rect.width) && (H == rect.height)) return;
        if (parent) { parent->refresh(); }
        rect.width = W;
        rect.height = H;
        eventResize();
    }

    void GWidget::setResource(shared_ptr<GResource> R) {
        resource = std::move(R);
        refresh();
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
       /* uint32_t idx;
        GWidget* p = parent;
        GWidget* s = this;

        auto it = children.begin();
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

    shared_ptr<Font>& GWidget::getFont() {
        return (font == nullptr ? window->getDefaultFont() : font);
    }

    void GWidget::init(GWidget&WND, AlignmentType ALIGN,
                       const string&RES, int32_t P) {
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

    shared_ptr<GWidget> GWidget::add(shared_ptr<GWidget>WND,
                                     AlignmentType ALIGN,
                                     const string RES,
                                     int32_t P) {
        assert(window && "GWidget must be added to a window before adding child");
        if (!allowChildren) return WND;
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
                eventMouseDown(MouseButton::MOUSE_BUTTON_LEFT,getRect().x, getRect().y);
                eventMouseUp(MouseButton::MOUSE_BUTTON_LEFT, getRect().x, getRect().y);
                break;
            default:
                call(TYP, std::move(EVT));
                break;
        }
    }

    bool GWidget::call(GEvent::Type TYP,  shared_ptr<GEvent> EVT) {
        const GWidget::GEventSlot &slot = slots[TYP];
        if (slot.func) {
            (slot.obj->*slot.func)(*this, EVT.get());
            return true;
        }
        return false;
    }

    void GWidget::eventCreate() {
        call(GEvent::OnCreate);
    }

    void GWidget::eventDestroy() {
        for (auto& child: children) {
            child->eventDestroy();
        }
        call(GEvent::OnDestroy);
        children.clear();
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
            if (parent) { parent->refresh(); }
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

    void GWidget::eventMove(int32_t X, int32_t Y) {
        int32_t diffX = rect.x - X;
        int32_t diffY = rect.y - Y;
        rect.x = X;
        rect.y = Y;
        for (auto& w: children) {
            w->setPos(w->rect.x - diffX, w->rect.y - diffY);
        }
        auto event = make_shared<GEventPos>(X, Y);
        call(GEvent::OnMove, event);
        if (parent) { parent->refresh(); }
        refresh();
    }

    void GWidget::eventResize() {
        if (freeze) return;
        freeze = true;
        if (parent) parent->resizeChildren();
        if (rect.width && rect.height) {
            auto event = make_shared<GEventSize>(rect.width, rect.height);
            resizeChildren();
            call(GEvent::OnResize, event);
        }
        refresh();
        freeze = false;
    }

    bool GWidget::clipRect(Rect&R, const Rect& R1, const Rect& R2) {
        if ((!R1.width) || (!R1.height) ||
            (!R2.width) || (!R2.height)) {
            return false;
        }
        if (R1.x > R2.x) {
            R.width = std::min(R1.width, R2.width - (R1.x - R2.x));
            R.x = R1.x;
        }
        else {
            R.width = std::min(R2.width, R1.width - (R2.x - R1.x));
            R.x = R2.x;
        }
        if (R1.y > R2.y) {
            R.height = std::min(R1.height, R2.height - (R1.y - R2.y));
            R.y = R1.y;
        }
        else {
            R.height = std::min(R2.height, R1.height - (R2.y - R1.y));
            R.y = R2.y;
        }
        return ((R.width > 0) && (R.height > 0));
    }

    void GWidget::maxRect(Rect&R, const Rect A, const Rect B) {
        if ((!A.width) || (!A.height)) {
            R = B;
        }
        else if ((!B.width) || (!B.height)) {
            R = A;
        }
        else {
            R.width = std::max(0, std::max(A.x + int32_t(A.width), B.x + int32_t(B.width)));
            R.height = std::max(0, std::max(A.y + int32_t(A.height), B.y + int32_t(B.height)));
            R.x = std::max(0, std::min(A.x, B.x));
            R.y = std::max(0, std::min(A.y, B.y));
            R.width -= R.x;
            R.height -= R.y;
        }
    }


    void GWidget::resizeChildren() {
        if ((!layout) || (freeze)) { return; }
        freeze = true;
        layout->resize(*this, rect, *resource);

        Rect clientRect = rect;
        clientRect.x += hborder + padding;
        if (clientRect.width > (2 * hborder + 2 * padding) ) {
            clientRect.width -= 2 * hborder + 2 * padding;
        }
        else {
            clientRect.width = 0;
        }
        clientRect.y += vborder + padding;
        if (clientRect.height > (2 * vborder + 2 * padding) ) {
            clientRect.height -= 2 * vborder + 2 * padding;
        }
        else {
            clientRect.height = 0;
        }
        if (pushed) {
            clientRect.x += 1;
            clientRect.y -= 1;
            clientRect.width -= 1;
            clientRect.height -= 1;
        }

        auto it = children.begin();
        while ((clientRect.width > 0) && (clientRect.height > 0) && (it != children.end())) {
            auto& child = *it;
            Rect childRect = child->rect;
            switch (child->alignment) {
                case FILL:
                    childRect = clientRect;
                    clientRect.width = 0;
                    clientRect.height = 0;
                    break;
                case CENTER:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.width = 0;
                    clientRect.height = 0;
                    break;
                case VCENTER:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.width = 0;
                    break;
                case HCENTER:
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    childRect.x = clientRect.x;
                    childRect.width = clientRect.width;
                    clientRect.width = 0;
                    break;
                case BOTTOM:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    childRect.width = clientRect.width;
                    clientRect.y += (childRect.height) + padding;
                    //arect.y = min(rect.height, arect.y + wrect.height + padding);
                    clientRect.height = std::max(0, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case LEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.x += (childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case TOP:
                    childRect.x = clientRect.x;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0, (clientRect.height - (childRect.height + 2 * padding)));
                    childRect.width = clientRect.width;
                    break;
                case RIGHT:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case BOTTOMCENTER:
                    childRect.y = clientRect.y;
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x +(clientRect.width - childRect.width) / 2;
                    clientRect.y +=(childRect.height) + padding;
                    //arect.y = min(rect.height, arect.y + wrect.height + padding);
                    clientRect.height -= childRect.height + 2 * padding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*padding)));
                    break;
                case TOPCENTER:
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    clientRect.height = std::max(0, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case LEFTCENTER:
                    childRect.x = clientRect.x;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.x +=(childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTCENTER:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case BOTTOMLEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    clientRect.y += (childRect.height) + padding;
                    //arect.y = min(rect.height, arect.y + wrect.height + padding);
                    clientRect.height -= childRect.height + 2 * padding;
                    //arect.height = max(0l, (arect.height - (wrect.height + 2*padding)));
                    break;
                case TOPLEFT:
                    childRect.x = clientRect.x;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case TOPRIGHT:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case BOTTOMRIGHT:
                    childRect.y = clientRect.y;
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    clientRect.height -= childRect.height + 2 * padding;
                    //arect.height = max(0l, (arect.height - (wrect.height + 2*padding)));
                    clientRect.y += (childRect.height) + padding;
                    //arect.y = min(rect.height, arect.y + wrect.height + padding);
                    break;
                case LEFTBOTTOM:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    clientRect.x += (childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case LEFTTOP:
                    childRect.x = clientRect.x;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.x += (childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTTOP:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTBOTTOM:
                    childRect.y = clientRect.y;
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    clientRect.width = std::max(0, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case CORNERBOTTOMLEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    clientRect.y += (childRect.height) + padding;
                    break;
                case CORNERTOPLEFT:
                    childRect.x = clientRect.x;
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    break;
                case CORNERTOPRIGHT:
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    if (childRect.height > clientRect.height)
                        childRect.y = 0;
                    else
                        childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    break;
                case CORNERBOTTOMRIGHT:
                    childRect.y = clientRect.y;
                    if (childRect.width > clientRect.width)
                        childRect.x = 0;
                    else
                        childRect.x = clientRect.x +(clientRect.width - childRect.width);
                    clientRect.y += (childRect.height) + padding;
                    break;
                default:
                    break;
            }
            child->rect = childRect;
            child->eventResize();
            ++it;
        }
        freeze = false;
    }

    bool GWidget::eventKeybDown(Key K) {
        if (!enabled) { return false; }
        auto event = make_shared<GEventKeyb>(K);
        call(GEvent::OnKeyDown, event); // XXX consumed
        return false;
    }

    bool GWidget::eventKeybUp(Key K) {
        if (!enabled) { return false; }
        if (focused) {
            auto event = make_shared<GEventKeyb>(K);
            call(GEvent::OnKeyUp, event); // XXX consumed
            return false;
        }
        return false;
    }

    bool GWidget::eventMouseDown(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) { return false; }
        bool consumed = false;
        bool r = redrawOnMouseEvent && (rect.contains(X, Y) || pushed);
        pushed = true;
        GWidget* wfocus = nullptr;
        for (auto& w : children) {
            if (w->getRect().contains(X, Y)) {
                consumed |= w->eventMouseDown(B, X, Y);
                wfocus = w.get();
                if (w->redrawOnMouseEvent) { w->refresh(); }
                if (consumed) { break; }
            }
        }
        if ((wfocus != nullptr) && (wfocus->allowFocus)) {
            wfocus->setFocus();
        }
        if (r && (!transparent)) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseDown, event);
        consumed |= event->consumed;
        return consumed;
    }

    bool GWidget::eventMouseUp(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) { return false; }
        bool consumed = false;
        bool r = redrawOnMouseEvent && (rect.contains(X, Y) || pushed);
        pushed = false;
        for (auto& w : children) {
            if (w->getRect().contains(X, Y)) {
                consumed |= w->eventMouseUp(B, X, Y);
                if (w->redrawOnMouseEvent) { w->refresh(); }
                if (consumed) { break; }
            }
        }
        if (r && (!transparent)) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseUp, event);
        consumed |= event->consumed;
        return consumed;
    }

    bool GWidget::eventMouseMove(MouseButton B, int32_t X, int32_t Y) {
        if (!enabled) { return false; }
        bool consumed = false;
        bool p = rect.contains(X, Y);
        for (auto& w : children) {
            p = w->getRect().contains(X, Y);
            if (w->redrawOnMouseMove && (w->pointed != p)) {
                w->pointed = p;
                w->refresh();
            }
            if (p) {
                consumed |= w->eventMouseMove(B, X, Y);
            } else if (w->pushed) {
                consumed |= w->eventMouseUp(B, X, Y);
            }
            if (consumed) { break; }
        }
        if (redrawOnMouseMove && (pointed != p) && (!transparent)) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMove, event);
        consumed |= event->consumed;
        return consumed;
    }

    void GWidget::eventGotFocus() {
        call(GEvent::OnGotFocus);
    }

    void GWidget::eventLostFocus() {
        call(GEvent::OnLostFocus);
    }

    void GWidget::setPadding(int32_t P) {
        padding = P;
        eventResize();
    }

    void GWidget::setDrawBackground(bool D) {
        drawBackground = D;
        refresh();
    }

    void GWidget::setAlignment(GWidget::AlignmentType ALIGN) {
        alignment = ALIGN;
        eventResize();
    }

    void GWidget::refresh() {
        if ((!freeze) && (window)){
            window->refresh();
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

    bool GWidget::isDrawBackground() const {
        return drawBackground;
    }

    Rect GWidget::getChildrenRect() const {
        return childrenRect;
    }

    bool GWidget::isPointed() const {
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

    const Rect& GWidget::getRect() const {
        return rect;
    }

    void GWidget::setRect(int32_t L, int32_t T, uint32_t W, uint32_t H) {
        setPos(L, T);
        setSize(W, H);
    }

    void GWidget::setRect(const Rect&R) {
        setRect(R.x, R.y, R.width, R.height);
    }

    bool GWidget::isPushed() const  {
        return pushed;
    }

    bool GWidget::isFreezed() const  {
        return freeze;
    }

    bool GWidget::isTransparent() const {
        return transparent;
    }

    bool GWidget::isRedrawOnMouseEvent() const {
        return redrawOnMouseEvent;
    }

    int32_t GWidget::getPadding() const {
        return padding;
    }

    int32_t GWidget::getVBorder() const {
        return vborder;
    }

    int32_t GWidget::getHBorder() const {
        return hborder;
    }

    void GWidget::setVBorder(int32_t B) {
        vborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    void GWidget::setHBorder(int32_t B) {
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