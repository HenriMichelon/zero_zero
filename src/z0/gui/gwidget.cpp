module;
#include <cassert>
#include "z0/libraries.h"

module Z0;

import :Font;
import :Rect;
import :Application;
import :VectorRenderer;
import :GWidget;
import :GResource;
import :GWindow;
import :GEvent;

namespace z0 {
    GWidget::GWidget(Type T): type{T} {
    }

    Application& GWidget::app() const {
        return Application::get();
    }

    void GWidget::_draw(VectorRenderer& R) const {
        if (!isVisible()) return;
        style->draw(*this, *resource, R, true);
        for (auto& child : children) {
            child->_draw(R);
        }
        style->draw(*this, *resource, R, false);
    }

    bool GWidget::isVisible() const {
        auto p = this;
        do {
            if (!p->visible) { return false; }
        } while ((p = p->parent) != nullptr);
        return (window && window->isVisible());
    }

    void GWidget::show(bool S) {
        if (visible == S) return;
        visible = S;
        if (visible) {
            eventShow();
        } else {
            eventHide();
        }
    }

    void GWidget::enable(bool S) {
        if (enabled == S) return;
        enabled = S;
        if (enabled) {
            eventEnable();
        } else {
            eventDisable();
        }
    }

    void GWidget::setPos(float x,
                         float y) {
        if ((x == rect.x) && (y == rect.y)) return;
        eventMove(x, y);
    }

    void GWidget::setSize(float W,
                          float H) {
        if (parent) { parent->refresh(); }
        if ((W != 0) && (H != 0) && (rect.width == 0) && (rect.height == 0)) {
            defaultRect = rect;
            defaultRect.width = W;
            defaultRect.height = H;
        }
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
        } else {
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
            auto event = GEvent{.source = this};
            if (F) {
                if (!freeze) { refresh(); }
                emit(GEvent::OnGotFocus, &event);
            } else {
                emit(GEvent::OnLostFocus, &event);
                /*shared_ptr<GWidget>p = parent;
                while (p && (!p->DrawBackground())) p = p->parent;
                if (p) { p->Refresh(rect); }*/
            }
        }
        return this;
    }

    void GWidget::allowingFocus(bool A) {
        allowFocus = A;
        for (const auto& child : children) {
            child->allowingFocus(false);
        }
    }

    shared_ptr<Font>& GWidget::getFont() {
        return (font == nullptr ? window->getDefaultFont() : font);
    }

    void GWidget::_init(GWidget& WND,
                        AlignmentType ALIGN,
                        const string& RES,
                        float P) {
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
            for (auto& child : W->_getChildren()) {
                W->remove(child);
            }
            children.remove(W);
            resizeChildren();
        }
        refresh();
    }

    void GWidget::removeAll() {
        for (const auto& child : children) {
            child->removeAll();
        }
        children.clear();
        refresh();
    }

    shared_ptr<GWidget> GWidget::add(shared_ptr<GWidget> WND,
                                     const AlignmentType ALIGN,
                                     const string& RES,
                                     const float P) {
        assert(window && "GWidget must be added to a window before adding child");
        if (!allowChildren) return WND;
        children.push_back(WND);
        _init(*WND, ALIGN, RES, P);
        return WND;
    }

    void GWidget::eventCreate() {
        auto event = GEvent{.source = this};
        emit(GEvent::OnCreate, &event);
    }

    void GWidget::eventDestroy() {
        for (const auto& child : children) {
            child->eventDestroy();
        }
        auto event = GEvent{.source = this};
        emit(GEvent::OnDestroy, &event);
        children.clear();
    }

    void GWidget::eventShow() {
        if (visible) {
            auto event = GEvent{.source = this};
            emit(GEvent::OnShow, &event);
            for (const auto& child : children) {
                child->eventShow();
            }
            if (parent) { refresh(); }
        }
    }

    void GWidget::eventHide() {
        if (!visible) {
            for (const auto& child : children) {
                child->eventHide();
            }
            if (parent) { parent->refresh(); }
            auto event = GEvent{.source = this};
            emit(GEvent::OnHide, &event);
        }
    }

    void GWidget::eventEnable() {
        auto event = GEvent{.source = this};
        emit(GEvent::OnEnable, &event);
        for (const auto& child : children) {
            child->enable();
        }
        refresh();
    }

    void GWidget::eventDisable() {
        for (const auto& child : children) {
            child->enable(false);
        }
        auto event = GEvent{.source = this};
        emit(GEvent::OnDisable, &event);
        refresh();
    }

    void GWidget::eventMove(const float X,
                            const float Y) {
        const float diffX = rect.x - X;
        const float diffY = rect.y - Y;
        rect.x = X;
        rect.y = Y;
        for (const auto& w : children) {
            w->setPos(w->rect.x - diffX, w->rect.y - diffY);
        }
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
        refresh();
        freeze = false;
    }

    void GWidget::resizeChildren() {
        if ((!style) || (freeze)) { return; }
        freeze = true;
        Rect r = getRect();
        style->resize(*this, r, *resource);

        Rect clientRect = rect;
        clientRect.x += hborder + padding;
        if (clientRect.width > (2 * hborder + 2 * padding)) {
            clientRect.width -= 2 * hborder + 2 * padding;
        } else {
            clientRect.width = 0;
        }
        clientRect.y += vborder + padding;
        if (clientRect.height > (2 * vborder + 2 * padding)) {
            clientRect.height -= 2 * vborder + 2 * padding;
        } else {
            clientRect.height = 0;
        }
        if (pushed && moveChildrenOnPush) {
            clientRect.x += 1;
            clientRect.y -= 1;
        }
        auto it = children.begin();
        while ((clientRect.width > 0) && (clientRect.height > 0) && (it != children.end())) {
            auto& child = *it;
            Rect childRect = child->_getDefaultRect();
            if (childRect.width > clientRect.width) {
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
                childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                clientRect.y += (childRect.height) + padding;
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
                clientRect.x += (childRect.width) + padding;
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
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                clientRect.y += (childRect.height) + padding;
                break;
            default:
                break;
            }
            child->setRect(childRect);
            ++it;
        }
        freeze = false;
    }

    bool GWidget::eventKeybDown(const Key K) {
        if (!enabled) { return false; }
        auto event = GEventKeyb{.key = K};
        event.source = this;
        emit(GEvent::OnKeyDown, &event);
        return event.consumed;
    }

    bool GWidget::eventKeybUp(const Key K) {
        if (!enabled) { return false; }
        if (focused) {
            auto event = GEventKeyb{.key = K};
            event.source = this;
            emit(GEvent::OnKeyUp, &event);
            return event.consumed;
        }
        return false;
    }

    bool GWidget::eventMouseDown(const MouseButton B,
                                 const float X,
                                 const float Y) {
        if (!enabled) { return false; }
        auto consumed = false;
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
        auto event = GEventMouseButton{.button = B, .x = X, .y = Y};
        event.source = this;
        emit(GEvent::OnMouseDown, &event);
        consumed |= event.consumed;
        return consumed;
    }

    bool GWidget::eventMouseUp(const MouseButton B,
                               const float X,
                               const float Y) {
        if (!enabled) { return false; }
        auto consumed = false;
        pushed = false;
        if (redrawOnMouseEvent) resizeChildren();
        for (const auto& w : children) {
            if (w->getRect().contains(X, Y) || w->isPushed()) {
                consumed |= w->eventMouseUp(B, X, Y);
                if (w->redrawOnMouseEvent) { w->refresh(); }
                if (consumed) { break; }
            }
        }
        if (redrawOnMouseEvent) { refresh(); }
        auto event = GEventMouseButton{.button = B, .x = X, .y = Y};
        event.source = this;
        emit(GEvent::OnMouseUp, &event);
        consumed |= event.consumed;
        return consumed;
    }

    bool GWidget::eventMouseMove(const uint32_t B,
                                 const float X,
                                 const float Y) {
        if (!enabled) { return false; }
        auto consumed = false;
        auto p = rect.contains(X, Y);
        for (auto& w : children) {
            p = w->getRect().contains(X, Y);
            if (w->redrawOnMouseMove && (w->pointed != p)) {
                w->pointed = p;
                w->refresh();
            }
            if (p) {
                consumed |= w->eventMouseMove(B, X, Y);
            } /*  else if (w->pushed) {
                consumed |= w->eventMouseUp(B, X, Y);
            } */
            if (consumed) { break; }
        }
        if (redrawOnMouseMove && (pointed != p)) { refresh(); }
        auto event = GEventMouseMove{.buttonsState = B, .x = X, .y = Y};
        event.source = this;
        emit(GEvent::OnMouseMove, &event);
        consumed |= event.consumed;
        return consumed;
    }

    void GWidget::eventGotFocus() {
        auto event = GEvent{.source = this};
        emit(GEvent::OnGotFocus, &event);
    }

    void GWidget::eventLostFocus() {
        auto event = GEvent{.source = this};
        emit(GEvent::OnLostFocus, &event);
    }

    void GWidget::setTransparency(const float alpha) {
        transparency = alpha;
        refresh();
    }

    void GWidget::setPadding(const float P) {
        padding = P;
        eventResize();
    }

    void GWidget::setDrawBackground(const bool D) {
        drawBackground = D;
        refresh();
    }

    void GWidget::setAlignment(const AlignmentType ALIGN) {
        alignment = ALIGN;
        eventResize();
    }

    void GWidget::refresh() const {
        if ((!freeze) && (window)) {
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

    void GWidget::setUserData(void* DATA) {
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

    void GWidget::setRect(const float L,
                          const float T,
                          const float W,
                          const float H) {
        setPos(L, T);
        setSize(W, H);
    }

    void GWidget::setRect(const Rect& R) {
        setRect(R.x, R.y, R.width, R.height);
    }

    bool GWidget::isPushed() const {
        return pushed;
    }

    bool GWidget::isFreezed() const {
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

    void GWidget::setVBorder(const float B) {
        vborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    void GWidget::setHBorder(const float B) {
        hborder = B;
        if (!freeze) { resizeChildren(); }
        refresh();
    }

    uint32_t GWidget::getGroupIndex() const {
        return groupIndex;
    }

    void* GWidget::getUserData() const {
        return userData;
    }
}
