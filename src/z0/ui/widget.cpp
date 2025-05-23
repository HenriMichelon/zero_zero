/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"

module z0.ui.Widget;

import z0.Application;
import z0.Constants;
import z0.Log;
import z0.Tools;

import z0.resources.Font;

import z0.ui.Event;
import z0.ui.Rect;
import z0.ui.Resource;
import z0.ui.Style;
import z0.ui.Window;

import z0.vulkan.VectorRenderer;

namespace z0::ui {

    Widget::Widget(const Type T) : type{T} {}

    void Widget::_draw(VectorRenderer &R) const {
        if (!isVisible()) {
            return;
        }
        const auto *s = static_cast<Style *>(style);
        s->draw(*this, *resource, R, true);
        for (auto &child : children) {
            child->_draw(R);
        }
        s->draw(*this, *resource, R, false);
    }

    bool Widget::isVisible() const {
        auto p = this;
        do {
            if (!p->visible) {
                return false;
            }
        } while ((p = p->parent) != nullptr);
        return (window && static_cast<Window *>(window)->isVisible());
    }

    void Widget::show(const bool S) {
        if (visible == S)
            return;
        visible = S;
        if (visible) {
            eventShow();
        } else {
            eventHide();
        }
    }

    void Widget::enable(const bool isEnabled) {
        if (enabled == isEnabled)
            return;
        enabled = isEnabled;
        if (enabled) {
            eventEnable();
        } else {
            eventDisable();
        }
    }

    void Widget::setPos(const float x, const float y) {
        if ((x == rect.x) && (y == rect.y)) { return; }
        eventMove(x, y);
    }

    void Widget::setSize(const float width, const float height) {
        if (parent) { parent->refresh(); }
        defaultRect.width  = width;
        defaultRect.height = height;
        rect.width  = width;
        rect.height = height;
        eventResize();
    }

    void Widget::_setSize(const float width, const float height) {
        if (parent) {
            parent->refresh();
        }
        if ((width != 0) && (height != 0) && (rect.width == 0) && (rect.height == 0)) {
            defaultRect        = rect;
            defaultRect.width  = width;
            defaultRect.height = height;
        }
        rect.width  = width;
        rect.height = height;
        eventResize();
    }

    void Widget::setResource(shared_ptr<Resource> R) {
        resource = std::move(R);
        refresh();
    }

    Widget *Widget::setNextFocus() {
        if (focused) {
            setFocus(false);
        } else {
            Widget *r = setFocus();
            if (r)
                return r;
        }

        if (!parent)
            return nullptr;
        /* uint32_t idx;
         Widget* p = parent;
         Widget* s = this;

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

    Widget *Widget::setFocus(const bool F) {
        if (!enabled)
            return nullptr;

        if (F && (!allowFocus)) {
            for (const auto &child : children) {
                const auto w = child->setFocus(F);
                if (w)
                    return w;
            }
            return nullptr;
        }

        if (focused != F) {
            focused    = F;
            auto event = Event{.source = this};
            if (F) {
                if (!freeze) {
                    refresh();
                }
                emit(Event::OnGotFocus, &event);
            } else {
                emit(Event::OnLostFocus, &event);
                /*shared_ptr<Widget>p = parent;
                while (p && (!p->DrawBackground())) p = p->parent;
                if (p) { p->Refresh(rect); }*/
            }
        }
        return this;
    }

    void Widget::allowingFocus(const bool A) {
        allowFocus = A;
        for (const auto &child : children) {
            child->allowingFocus(false);
        }
    }

    Font &Widget::getFont() { return (font ? *font : app().getWindowManager().getDefaultFont()); }

    void Widget::_init(Widget &child, const AlignmentType alignment, const string &res, const bool overlap) {
        child.alignment = alignment;
        child.overlap   = overlap;
        if (!child.font) {
            child.font = font;
        }
        child.window = window;
        child.style  = style;
        child.parent = this;
        static_cast<Style *>(style)->addResource(child, res);
        child.eventCreate();
        child.freeze = false;
        if (static_cast<Window *>(window)->isVisible() && (resource != nullptr)) {
            resizeChildren();
        }
    }

    void Widget::remove(const shared_ptr<Widget>& W) {
        const auto it = ranges::find(children, W);
        if (it != children.end()) {
            W->parent = nullptr;
            // for (const auto& child : W->_getChildren()) {
            //     W->remove(child);
            // }
            children.remove(W);
            resizeChildren();
        }
        refresh();
    }

    void Widget::removeAll() {
        for (const auto &child : children) {
            child->removeAll();
        }
        children.clear();
        refresh();
    }

    void Widget::eventCreate() {
        auto event = Event{.source = this};
        emit(Event::OnCreate, &event);
    }

    void Widget::eventDestroy() {
        for (const auto &child : children) {
            child->eventDestroy();
        }
        auto event = Event{.source = this};
        emit(Event::OnDestroy, &event);
        children.clear();
    }

    void Widget::eventShow() {
        if (visible) {
            auto event = Event{.source = this};
            emit(Event::OnShow, &event);
            for (const auto &child : children) {
                child->eventShow();
            }
            if (parent) {
                refresh();
            }
        }
    }

    void Widget::eventHide() {
        if (!visible) {
            for (const auto &child : children) {
                child->eventHide();
            }
            if (parent) {
                parent->refresh();
            }
            auto event = Event{.source = this};
            emit(Event::OnHide, &event);
        }
    }

    void Widget::eventEnable() {
        auto event = Event{.source = this};
        emit(Event::OnEnable, &event);
        for (const auto &child : children) {
            child->enable();
        }
        refresh();
    }

    void Widget::eventDisable() {
        for (const auto &child : children) {
            child->enable(false);
        }
        auto event = Event{.source = this};
        emit(Event::OnDisable, &event);
        refresh();
    }

    void Widget::eventMove(const float X, const float Y) {
        const float diffX = rect.x - X;
        const float diffY = rect.y - Y;
        rect.x            = X;
        rect.y            = Y;
        for (const auto &w : children) {
            w->setPos(w->rect.x - diffX, w->rect.y - diffY);
        }
        if (parent) {
            parent->refresh();
        }
        refresh();
    }

    void Widget::eventResize() {
        if (freeze) { return; }
        if (parent) { parent->resizeChildren(); }
        resizeChildren();
        freeze = true;
        refresh();
        freeze = false;
    }

    void Widget::resizeChildren() {
        if (!style || freeze) {
            return;
        }
        freeze = true;
        Rect r = getRect();
        static_cast<Style *>(style)->resize(*this, r, *resource);

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
            auto &child     = *it;
            Rect  childRect = child->_getDefaultRect();
            if (childRect.width > (clientRect.width)) {
                childRect.width = clientRect.width;
            }
            if (childRect.height > (clientRect.height)) {
                childRect.height = clientRect.height;
            }
            switch (child->alignment) {
            case FILL:
                childRect = clientRect;
                if (!child->overlap) {
                    clientRect.width  = 0;
                    clientRect.height = 0;
                }
                break;
            case CENTER:
                childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                if (!child->overlap) {
                    clientRect.width  = 0;
                    clientRect.height = 0;
                }
                break;
            case VCENTER:
                childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    childRect.height = clientRect.height;
                    clientRect.width = 0;
                }
                break;
            case HCENTER:
                childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                childRect.x = clientRect.x;
                if (!child->overlap) {
                    childRect.width  = clientRect.width;
                    clientRect.width = 0;
                }
                break;
            case BOTTOM:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    clientRect.y += (childRect.height);
                    childRect.width   = clientRect.width;
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding)));
                }
                break;
            case LEFT:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    clientRect.x += childRect.width + 1;
                    childRect.height = clientRect.height;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case TOP:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                if (!child->overlap) {
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding) - 1));
                    childRect.width   = clientRect.width;
                }
                break;
            case RIGHT:
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    childRect.height = clientRect.height;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case BOTTOMCENTER:
                childRect.y = clientRect.y;
                childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                if (!child->overlap) {
                    clientRect.y += (childRect.height) + padding;
                    clientRect.height -= childRect.height + 2 * padding;
                }
                break;
            case TOPCENTER:
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                childRect.x = clientRect.x + (clientRect.width - childRect.width) / 2;
                if (!child->overlap) {
                    clientRect.height = std::max(0.0f, clientRect.height - (childRect.height + 2 * padding) - 1);
                }
                break;
            case LEFTCENTER:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                if (!child->overlap) {
                    clientRect.x += (childRect.width) + padding + 1;
                    clientRect.width = std::max(0.0f, clientRect.width - (childRect.width + 2 * padding) - 1);
                }
                break;
            case RIGHTCENTER:
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                childRect.y = clientRect.y + (clientRect.height - childRect.height) / 2;
                if (!child->overlap) {
                    clientRect.width = std::max(0.0f, clientRect.width - (childRect.width + 2 * padding) - 1);
                }
                break;
            case BOTTOMLEFT:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    clientRect.y += (childRect.height) + padding;
                    clientRect.height -= childRect.height + 2 * padding;
                }
                break;
            case TOPLEFT:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                if (!child->overlap) {
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding) - 1));
                }
                break;
            case TOPRIGHT:
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                if (!child->overlap) {
                    clientRect.height = std::max(0.0f, (clientRect.height - (childRect.height + 2 * padding) - 1));
                }
                break;
            case BOTTOMRIGHT:
                childRect.y = clientRect.y;
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                if (!child->overlap) {
                    clientRect.height -= childRect.height + 2 * padding;
                    clientRect.y += (childRect.height) + padding;
                }
                break;
            case LEFTBOTTOM:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    clientRect.x += childRect.width + padding + 1;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case LEFTTOP:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                if (!child->overlap) {
                    clientRect.x += childRect.width + padding + 1;
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case RIGHTTOP:
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                childRect.y = clientRect.y + (clientRect.height - childRect.height);
                if (!child->overlap) {
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case RIGHTBOTTOM:
                childRect.y = clientRect.y;
                childRect.x = clientRect.x + (clientRect.width - childRect.width);
                if (!child->overlap) {
                    clientRect.width = std::max(0.0f, (clientRect.width - (childRect.width + 2 * padding) - 1));
                }
                break;
            case CORNERBOTTOMLEFT:
                childRect.x = clientRect.x;
                childRect.y = clientRect.y;
                if (!child->overlap) {
                    clientRect.y += (childRect.height) + padding;
                }
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
                if (!child->overlap) {
                    clientRect.y += (childRect.height) + padding;
                }
                break;
            default:
                break;
            }
            child->setRect(childRect);
            ++it;
        }
        freeze = false;
    }

    bool Widget::eventKeybDown(const Key K) {
        if (!enabled) {
            return false;
        }
        auto event   = EventKeyb{.key = K};
        event.source = this;
        emit(Event::OnKeyDown, &event);
        return event.consumed;
    }

    bool Widget::eventKeybUp(const Key K) {
        if (!enabled) {
            return false;
        }
        if (focused) {
            auto event   = EventKeyb{.key = K};
            event.source = this;
            emit(Event::OnKeyUp, &event);
            return event.consumed;
        }
        return false;
    }

    bool Widget::eventMouseDown(const MouseButton B, const float X, const float Y) {
        if (!enabled) {
            return false;
        }
        auto consumed = false;
        pushed        = true;
        if (redrawOnMouseEvent)
            resizeChildren();
        Widget *wfocus = nullptr;
        for (auto &w : children) {
            if (w->getRect().contains(X, Y)) {
                consumed |= w->eventMouseDown(B, X, Y);
                wfocus = w.get();
                if (w->redrawOnMouseEvent) {
                    w->refresh();
                }
                if (consumed) {
                    break;
                }
            }
        }
        if ((wfocus != nullptr) && (wfocus->allowFocus)) {
            wfocus->setFocus();
        }
        if (redrawOnMouseEvent) {
            refresh();
        }
        auto event   = EventMouseButton{.button = B, .x = X, .y = Y};
        event.source = this;
        emit(Event::OnMouseDown, &event);
        consumed |= event.consumed;
        return consumed;
    }

    bool Widget::eventMouseUp(const MouseButton B, const float X, const float Y) {
        if (!enabled) {
            return false;
        }
        auto consumed = false;
        pushed        = false;
        if (redrawOnMouseEvent)
            resizeChildren();
        for (const auto &w : children) {
            if (w->getRect().contains(X, Y) || w->isPushed()) {
                consumed |= w->eventMouseUp(B, X, Y);
                if (w->redrawOnMouseEvent) {
                    w->refresh();
                }
                if (consumed) {
                    break;
                }
            }
        }
        if (redrawOnMouseEvent) {
            refresh();
        }
        auto event   = EventMouseButton{.button = B, .x = X, .y = Y};
        event.source = this;
        emit(Event::OnMouseUp, &event);
        consumed |= event.consumed;
        return consumed;
    }

    bool Widget::eventMouseMove(const uint32_t B, const float X, const float Y) {
        if (!enabled) {
            return false;
        }
        auto consumed = false;
        auto p        = rect.contains(X, Y);
        for (auto &w : children) {
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
            if (consumed) {
                break;
            }
        }
        if (redrawOnMouseMove && (pointed != p)) {
            refresh();
        }
        auto event   = EventMouseMove{.buttonsState = B, .x = X, .y = Y};
        event.source = this;
        emit(Event::OnMouseMove, &event);
        consumed |= event.consumed;
        return consumed;
    }

    void Widget::eventGotFocus() {
        auto event = Event{.source = this};
        emit(Event::OnGotFocus, &event);
    }

    void Widget::eventLostFocus() {
        auto event = Event{.source = this};
        emit(Event::OnLostFocus, &event);
    }

    void Widget::setTransparency(const float alpha) {
        transparency = alpha;
        refresh();
    }

    void Widget::setPadding(const float P) {
        padding = P;
        eventResize();
    }

    void Widget::setDrawBackground(const bool D) {
        drawBackground = D;
        refresh();
    }

    void Widget::setAlignment(const AlignmentType ALIGN) {
        alignment = ALIGN;
        eventResize();
    }

    void Widget::refresh() const {
        if ((!freeze) && (window)) {
            static_cast<Window *>(window)->refresh();
        }
    }

    void Widget::setFont(const shared_ptr<Font> &F) {
        font = F;
        resizeChildren();
        refresh();
    }

    void Widget::setGroupIndex(const int32_t IDX) { groupIndex = IDX; }

    void Widget::setUserData(void *DATA) { userData = DATA; }

    bool Widget::isDrawBackground() const { return drawBackground; }

    Rect Widget::getChildrenRect() const { return childrenRect; }

    bool Widget::isPointed() const { return pointed; }

    Widget::AlignmentType Widget::getAlignment() const { return alignment; }

    Widget::Type Widget::getType() const { return type; }

    bool Widget::isEnabled() const { return enabled; }

    bool Widget::isFocused() const { return focused; }

    const Rect &Widget::getRect() const { return rect; }

    void Widget::setRect(const float L, const float T, const float W, const float H) {
        setPos(L, T);
        _setSize(W, H);
    }

    void Widget::setRect(const Rect &R) { setRect(R.x, R.y, R.width, R.height); }

    bool Widget::isPushed() const { return pushed; }

    bool Widget::isFreezed() const { return freeze; }

    bool Widget::isRedrawOnMouseEvent() const { return redrawOnMouseEvent; }

    float Widget::getPadding() const { return padding; }

    float Widget::getVBorder() const { return vborder; }

    float Widget::getHBorder() const { return hborder; }

    void Widget::setVBorder(const float B) {
        vborder = B;
        if (!freeze) {
            resizeChildren();
        }
        refresh();
    }

    void Widget::setHBorder(const float B) {
        hborder = B;
        if (!freeze) {
            resizeChildren();
        }
        refresh();
    }

    uint32_t Widget::getGroupIndex() const { return groupIndex; }

    void *Widget::getUserData() const { return userData; }
} // namespace z0::ui
