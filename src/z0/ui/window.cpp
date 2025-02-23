/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include "z0/libraries.h"

module z0.ui.Window;

import z0.Application;
import z0.Constants;
import z0.resources.Font;

import z0.ui.Event;
import z0.ui.Manager;
import z0.ui.Panel;
import z0.ui.Rect;
import z0.ui.Style;
import z0.ui.Widget;

import z0.vulkan.VectorRenderer;

namespace z0::ui {

    Window::Window(const Rect& rect): rect{rect} {}

    void Window::draw() const {
        if (!isVisible()) { return; }
        const auto *wm = static_cast<Manager *>(windowManager);
        wm->getRenderer().setTranslate({rect.x, rect.y});
        wm->getRenderer().setTransparency(1.0f - transparency);
        widget->_draw(wm->getRenderer());
    }

    void Window::unFreeze(const shared_ptr<Widget> &widget) {
        for (auto &child : widget->_getChildren()) {
            unFreeze(child);
        }
        widget->setFreezed(false);
    }

    Font &Window::getDefaultFont() const {
        const auto *wm = static_cast<Manager *>(windowManager);
        return wm->getDefaultFont();
    }

    void Window::setWidget(shared_ptr<Widget> child, const string &resources, const float padding) {
        assert(windowManager && "ui::Window must be added to a Window manager before setting the main widget");
        if (layout == nullptr) { setStyle(nullptr); }
        if (widget == nullptr) {
            widget = make_shared<Panel>();
            widget->setDrawBackground(false);
        } else {
            widget = std::move(child);
        }
        widget->setFreezed( true);
        widget->setPadding(padding);
        widget->window  = this;
        widget->style   = layout.get();
        widget->setFont(static_cast<Style*>(widget->style)->getFont());
        static_cast<Style*>(widget->style)->addResource(*widget, resources);
        widget->eventCreate();
        widget->setPos(0, 0);
        widget->setSize(getWidth(), getHeight());
        focusedWidget = widget->setFocus();
        unFreeze(widget);
    }

    void Window::setStyle(const shared_ptr<Style>& style) {
        if (layout == nullptr) {
            layout = Style::create();
        } else {
            layout = std::move(style);
        }
        refresh();
    }

    void Window::setVisible(const bool isVisible) {
        if (visible != isVisible) {
            visibilityChange  = isVisible;
            visibilityChanged = true;
        }
    }

    void Window::hide() {
        setVisible(false);
    }

    void Window::show() {
        setVisible(true);
    }

    void Window::eventCreate() {
        setWidget();
        onCreate();
        emit(Event::OnCreate);
        if (widget != nullptr) { widget->resizeChildren(); }
    }

    void Window::eventDestroy() {
        if (widget) { widget->eventDestroy(); }
        emit(Event::OnDestroy);
        onDestroy();
        widget.reset();
    }

    void Window::eventShow() {
        if (widget) { widget->eventShow(); }
        onShow();
        emit(Event::OnShow);
        refresh();
    }

    bool Window::eventKeybDown(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            consumed = focusedWidget->eventKeybDown(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyDown(K);
        }
        if (!consumed) {
            auto event = EventKeyb{.key = K};
            emit(Event::OnKeyDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventKeybUp(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            focusedWidget->eventKeybUp(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyUp(K);
        }
        if (!consumed) {
            auto event = EventKeyb{.key = K};
            emit(Event::OnKeyUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseDown(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) {
            consumed = widget->eventMouseDown(B, X, Y);
        }
        if (!consumed) {
            consumed |= onMouseDown(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseButton{.button = B, .x = X, .y = Y};
            emit(Event::OnMouseDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseUp(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) { consumed = widget->eventMouseUp(B, X, Y); }
        if (!consumed) {
            consumed |= onMouseUp(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseButton{.button = B, .x = X, .y = Y};
            emit(Event::OnMouseUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool Window::eventMouseMove(const uint32_t B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if ((focusedWidget != nullptr) &&
            (focusedWidget->mouseMoveOnFocus)) {
            consumed = focusedWidget->eventMouseMove(B, X, Y);
            } else if (widget) {
                consumed = widget->eventMouseMove(B, X, Y);
            }
        if (!consumed) {
            consumed |= onMouseMove(B, X, Y);
        }
        if (!consumed) {
            auto event = EventMouseMove{.buttonsState = B, .x = X, .y = Y};
            emit(Event::OnMouseMove, &event);
            consumed = event.consumed;
        }
        if (consumed) { refresh(); }
        return consumed;
    }

    void Window::refresh() const {
        if (windowManager) { static_cast<Manager*>(windowManager)->refresh(); }
    }

    void Window::setFocusedWidget(const shared_ptr<Widget> &W) {
        focusedWidget = W.get();
    }

    Widget &Window::getWidget() const {
        assert(windowManager && "Window must be added to a Window manager before use");
        return *widget;
    }

    void Window::setRect(const Rect &newRect) {
        rect        = newRect;
        rect.width  = std::min(std::max(newRect.width, minWidth), maxWidth);
        rect.height = std::min(std::max(newRect.height, minHeight), maxHeight);
        eventResize();
    }

    void Window::setHeight(const float h) {
        rect.height = std::min(std::max(h, minHeight), maxHeight);
        eventResize();
    }

    void Window::setWidth(const float w) {
        rect.width = std::min(std::max(w, minWidth), maxWidth);
        eventResize();
    }

    void Window::setPos(const float x, const float y) {
        rect.x = x;
        rect.y = y;
        eventMove();
    }

    void Window::setPos(const vec2 pos) {
        rect.x = pos.x;
        rect.y = pos.y;
        eventMove();
    }

    void Window::setX(const float x) {
        rect.x = x;
        eventMove();
    }

    void Window::setY(const float y) {
        rect.y = y;
        eventMove();
    }

    shared_ptr<Style> Window::getStyle() const {
        return layout;
    }

    void Window::setTransparency(const float alpha) {
        transparency = alpha;
        refresh();
    }

    void Window::eventResize() {
        if (widget) { widget->setSize(rect.width, rect.height); }
        onResize();
        emit(Event::OnResize);
        refresh();
    }

    void Window::eventMove() {
        if (widget) { widget->resizeChildren(); }
        onMove();
        emit(Event::OnMove);
        refresh();
    }

    void Window::eventHide() {
        emit(Event::OnHide);
        onHide();
        refresh();
    }

    void Window::eventGotFocus() {
        onGotFocus();
        emit(Event::OnGotFocus);
        refresh();
    }

    void Window::eventLostFocus() {
        onLostFocus();
        emit(Event::OnLostFocus);
        refresh();
    }

    void Window::setMinimumSize(const float width, const float height) {
        minWidth  = width;
        minHeight = height;
    }

    void Window::setMaximumSize(const float width, const float height) {
        maxWidth  = width;
        maxHeight = height;
    }
}

