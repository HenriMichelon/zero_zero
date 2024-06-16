#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gwindow.h"
#endif

namespace z0 {

    GWidget::GWidget(Type T): type{T} {
    }

        Application& GWidget::app() {
        return Application::get();
    }

    void GWidget::draw(VectorRenderer &R) const {
        if (!isVisible()) return;
        style->draw(*this, *resource, R, true);
        for(auto& child: children) {
            child->draw(R);
        }
        style->draw(*this, *resource, R, false);
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

    void GWidget::setPos(float x, float y) {
        if ((x == rect.x) && (y == rect.y)) return;
        eventMove(x, y);
    }

    void GWidget::setSize(float W, float H) {
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

    void GWidget::init(GWidget&WND, AlignmentType ALIGN, const string&RES, float P) {
        WND.padding = P;
        WND.alignment = ALIGN;
        if (!WND.font) { WND.font = font; }
        WND.window = window;
        WND.style = style;
        WND.parent = this;
        style->addResource(WND, RES);
        WND.eventCreate();
        WND.freeze = false;
        if (window->isVisible() && (resource != nullptr)) {
            resizeChildren();
        }
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
                                     float P) {
        assert(window && "GWidget must be added to a window before adding child");
        if (!allowChildren) return WND;
        children.push_back(WND);
        init(*WND, ALIGN, RES, P);
        return WND;
    }

    void GWidget::connect(GEvent::Type TYP,
                          GEventHandler* OBJ,
                          const GEventFunction FUNC) {
        //cassert(OBJ, "Invalid object for event slot connection");
        //PRE(FUNC, "Invalid method for event slot connection");
        slots[TYP].obj = OBJ;
        slots[TYP].func = FUNC;
    }

    bool GWidget::call(GEvent::Type TYP,  shared_ptr<GEvent> EVT) {
        const GEventSlot &slot = slots[TYP];
        if (slot.func) {
            (slot.obj->*slot.func)(this, EVT.get());
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

    void GWidget::eventMove(float X, float Y) {
        float diffX = rect.x - X;
        float diffY = rect.y - Y;
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
        if (parent) {
            parent->resizeChildren();
        }
        resizeChildren();
        freeze = true;
        if ((rect.width != 0.0f) && (rect.height != 0.0f)) {
            auto event = make_shared<GEventSize>(rect.width, rect.height);
            call(GEvent::OnResize, event);
        }
        refresh();
        freeze = false;
    }

    bool GWidget::clipRect(Rect&R, const Rect& R1, const Rect& R2) {
        if ((R1.width == 0.0f) || (R1.height == 0.0f) ||
            (R2.width == 0.0f) || (R2.height == 0.0f)) {
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
        if ((A.width == 0.0f) || (A.height == 0.0f)) {
            R = B;
        }
        else if ((B.width == 0.0f) || (B.height == 0.0f)) {
            R = A;
        }
        else {
            R.width = std::max(0.0f, std::max(A.x + A.width, B.x + B.width));
            R.height = std::max(0.0f, std::max(A.y + A.height, B.y + B.height));
            R.x = std::max(0.0f, std::min(A.x, B.x));
            R.y = std::max(0.0f, std::min(A.y, B.y));
            R.width -= R.x;
            R.height -= R.y;
        }
    }


    void GWidget::resizeChildren() {
        if ((!style) || (freeze)) { return; }
        freeze = true;
        style->resize(*this, rect, *resource);

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
        if (pushed && moveChildsOnPush) {
            clientRect.x += 1;
            clientRect.y -= 1;
        }
        auto it = children.begin();
        while ((clientRect.width > 0) && (clientRect.height > 0) && (it != children.end())) {
            auto& child = *it;
            Rect childRect = child->rect;
            if (childRect.width > clientRect.width)  {
                childRect.width = clientRect.width;
            }
            if (childRect.height > clientRect.height) {
                childRect.height = clientRect.height;
            }
            switch (child->alignment) {
                case FILL:
                    childRect = clientRect;
                    clientRect.width = 0;
                    clientRect.height = 0;
                    break;
                case CENTER:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.width = 0;
                    clientRect.height = 0;
                    break;
                case VCENTER:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.width = 0;
                    break;
                case HCENTER:
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
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case LEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.x += (childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case TOP:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                    childRect.width = clientRect.width;
                    break;
                case RIGHT:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y;
                    childRect.height = clientRect.height;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case BOTTOMCENTER:
                    childRect.y = clientRect.y;
                    childRect.x = clientRect.x +(clientRect.width - childRect.width) / 2;
                    clientRect.y +=(childRect.height) + padding;
                    //arect.y = min(rect.height, arect.y + wrect.height + padding);
                    clientRect.height -= childRect.height + 2 * padding;
                    //arect.height = max(0l, int32_t(arect.height - (wrect.height + 2*padding)));
                    break;
                case TOPCENTER:
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case LEFTCENTER:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.x +=(childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTCENTER:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
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
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case TOPRIGHT:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                    break;
                case BOTTOMRIGHT:
                    childRect.y = clientRect.y;
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
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case LEFTTOP:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.x += (childRect.width) + padding;
                    //arect.x = min(rect.width, arect.x + wrect.width + padding);
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTTOP:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case RIGHTBOTTOM:
                    childRect.y = clientRect.y;
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding)));
                    break;
                case CORNERBOTTOMLEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y;
                    clientRect.y += (childRect.height) + padding;
                    break;
                case CORNERTOPLEFT:
                    childRect.x = clientRect.x;
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    break;
                case CORNERTOPRIGHT:
                    childRect.x = clientRect.x + (clientRect.width - childRect.width);
                    childRect.y = clientRect.y + (clientRect.height - childRect.height);
                    break;
                case CORNERBOTTOMRIGHT:
                    childRect.y = clientRect.y;
                    childRect.x = clientRect.x +(clientRect.width - childRect.width);
                    clientRect.y += (childRect.height) + padding;
                    break;
                default:
                    break;
            }
            child->setRect(childRect);
            //child->rect = childRect;
            //child->eventResize();
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

    bool GWidget::eventMouseDown(MouseButton B, float X, float Y) {
        if (!enabled) { return false; }
        bool consumed = false;
        pushed = true;
        if (redrawOnMouseEvent) resizeChildren();
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
        if (redrawOnMouseEvent) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseDown, event);
        consumed |= event->consumed;
        return consumed;
    }

    bool GWidget::eventMouseUp(MouseButton B, float X, float Y) {
        if (!enabled) { return false; }
        bool consumed = false;
        pushed = false;
        if (redrawOnMouseEvent) resizeChildren();
        for (auto& w : children) {
            if (w->getRect().contains(X, Y) || w->isPushed()) {
                consumed |= w->eventMouseUp(B, X, Y);
                if (w->redrawOnMouseEvent) { w->refresh(); }
                if (consumed) { break; }
            }
        }
        if (redrawOnMouseEvent) { refresh(); }
        auto event = make_shared<GEventMouse>(B, X, Y);
        call(GEvent::OnMouseUp, event);
        consumed |= event->consumed;
        return consumed;
    }

    bool GWidget::eventMouseMove(MouseButton B, float X, float Y) {
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
        if (redrawOnMouseMove && (pointed != p)) { refresh(); }
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

    void GWidget::setTransparency(float alpha) {
        transparency = alpha;
        refresh();
    }

    void GWidget::setPadding(float P) {
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

    void GWidget::setFont(const shared_ptr<Font>& F) {
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

    void GWidget::setRect(float L, float T, float W, float H) {
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

    bool GWidget::isRedrawOnMouseEvent() const {
        return redrawOnMouseEvent;
    }

    float GWidget::getPadding() const {
        return padding;
    }

    float GWidget::getVBorder() const {
        return vborder;
    }

    float GWidget::getHBorder() const {
        return hborder;
    }

    void GWidget::setVBorder(float B) {
        vborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    void GWidget::setHBorder(float B) {
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