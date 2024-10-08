module;
#include <cassert>
#include "z0/libraries.h"

module z0;

import :Rect;
import :Font;
import :Application;
import :GEvent;
import :GManager;
import :GWindow;
import :GWidget;
import :GPanel;
import :GStyle;
import :VectorRenderer;

namespace z0 {
    GWindow::GWindow(Rect r):
        rect{r} {
    }

    Application &GWindow::app() const {
        return Application::get();
    }

    void GWindow::draw() const {
        if (!isVisible())
            return;
        windowManager->getRenderer().setTranslate({rect.x, rect.y});
        windowManager->getRenderer().setTransparency(1.0f - transparency);
        widget->_draw(windowManager->getRenderer());
    }

    void GWindow::unFreeze(shared_ptr<GWidget> &W) {
        for (auto &child : W->_getChildren()) {
            unFreeze(child);
        }
        W->setFreezed(false);
    }

    shared_ptr<Font> &GWindow::getDefaultFont() const {
        return windowManager->getDefaultFont();
    }

    GWidget &GWindow::setWidget(shared_ptr<GWidget> WIDGET, const string &RES, float PADDING) {
        assert(windowManager && "GWidow must be added to a window manager before setting the main widget");
        if (layout == nullptr) { setStyle(nullptr); }
        if (WIDGET == nullptr) {
            widget = make_shared<GPanel>();
        } else {
            widget = std::move(WIDGET);
        }
        widget->freeze  = true;
        widget->padding = PADDING;
        widget->window  = this;
        widget->style   = layout;
        widget->font    = widget->style->getFont();
        widget->style->addResource(*widget, RES);
        widget->setDrawBackground(true);
        widget->eventCreate();
        widget->setPos(0, 0);
        widget->setSize(getWidth(), getHeight());
        focusedWidget = widget->setFocus();
        unFreeze(widget);
        return *widget;
    }

    void GWindow::setStyle(shared_ptr<GStyle> LAYOUT) {
        if (layout == nullptr) {
            layout = GStyle::create();
        } else {
            layout = std::move(LAYOUT);
        }
        refresh();
    }

    void GWindow::setVisible(const bool isVisible) {
        if (visible != isVisible) {
            visibilityChange  = isVisible;
            visibilityChanged = true;
        }
    }

    void GWindow::hide() {
        setVisible(false);
    }

    void GWindow::show() {
        setVisible(true);
    }

    void GWindow::eventCreate() {
        setWidget();
        onCreate();
        emit(GEvent::OnCreate);
        if (widget != nullptr) { widget->resizeChildren(); }
    }

    void GWindow::eventDestroy() {
        if (widget) { widget->eventDestroy(); }
        emit(GEvent::OnDestroy);
        onDestroy();
        widget.reset();
    }

    void GWindow::eventShow() {
        if (widget) { widget->eventShow(); }
        onShow();
        emit(GEvent::OnShow);
        refresh();
    }

    bool GWindow::eventKeybDown(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            consumed = focusedWidget->eventKeybDown(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyDown(K);
        }
        if (!consumed) {
            auto event = GEventKeyb{.key = K};
            emit(GEvent::OnKeyDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool GWindow::eventKeybUp(const Key K) {
        bool consumed = false;
        if (focusedWidget) {
            focusedWidget->eventKeybUp(K); // XXX consumed
        }
        if (!consumed) {
            consumed |= onKeyUp(K);
        }
        if (!consumed) {
            auto event = GEventKeyb{.key = K};
            emit(GEvent::OnKeyUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseDown(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) {
            consumed = widget->eventMouseDown(B, X, Y);
        }
        if (!consumed) {
            consumed |= onMouseDown(B, X, Y);
        }
        if (!consumed) {
            auto event = GEventMouseButton{.button = B, .x = X, .y = Y};
            emit(GEvent::OnMouseDown, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseUp(const MouseButton B, const float X, const float Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) { consumed = widget->eventMouseUp(B, X, Y); }
        if (!consumed) {
            consumed |= onMouseUp(B, X, Y);
        }
        if (!consumed) {
            auto event = GEventMouseButton{.button = B, .x = X, .y = Y};
            emit(GEvent::OnMouseUp, &event);
            consumed = event.consumed;
        }
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseMove(const uint32_t B, const float X, const float Y) {
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
            auto event = GEventMouseMove{.buttonsState = B, .x = X, .y = Y};
            emit(GEvent::OnMouseMove, &event);
            consumed = event.consumed;
        }
        if (consumed) { refresh(); }
        return consumed;
    }

    void GWindow::refresh() {
        if (windowManager) { windowManager->refresh(); }
    }

    void GWindow::setFocusedWidget(const shared_ptr<GWidget> &W) {
        focusedWidget = W.get();
    }

    GWidget &GWindow::getWidget() const {
        assert(windowManager && "GWindow must be added to a window manager before use");
        return *widget;
    }

    void GWindow::setRect(const Rect &r) {
        rect        = r;
        rect.width  = std::min(std::max(r.width, minWidth), maxWidth);
        rect.height = std::min(std::max(r.height, minHeight), maxHeight);
        eventResize();
    }

    void GWindow::setHeight(const float h) {
        rect.height = std::min(std::max(h, minHeight), maxHeight);
        eventResize();
    }

    void GWindow::setWidth(const float w) {
        rect.width = std::min(std::max(w, minWidth), maxWidth);
        eventResize();
    }

    void GWindow::setPos(const float x, const float y) {
        rect.x = x;
        rect.y = y;
        eventMove();
    }

    void GWindow::setPos(const vec2 pos) {
        rect.x = pos.x;
        rect.y = pos.y;
        eventMove();
    }

    void GWindow::setX(const float x) {
        rect.x = x;
        eventMove();
    }

    void GWindow::setY(const float y) {
        rect.y = y;
        eventMove();
    }

    shared_ptr<GStyle> GWindow::getStyle() const {
        return layout;
    }

    void GWindow::setTransparency(const float alpha) {
        transparency = alpha;
        refresh();
    }

    void GWindow::eventResize() {
        if (widget) { widget->setSize(rect.width, rect.height); }
        onResize();
        emit(GEvent::OnResize);
        refresh();
    }

    void GWindow::eventMove() {
        if (widget) { widget->resizeChildren(); }
        onMove();
        emit(GEvent::OnMove);
        refresh();
    }

    void GWindow::eventHide() {
        emit(GEvent::OnHide);
        onHide();
        refresh();
    }

    void GWindow::eventGotFocus() {
        onGotFocus();
        emit(GEvent::OnGotFocus);
        refresh();
    }

    void GWindow::eventLostFocus() {
        onLostFocus();
        emit(GEvent::OnLostFocus);
        refresh();
    }

    void GWindow::setMinimumSize(const float width, const float height) {
        minWidth  = width;
        minHeight = height;
    }

    void GWindow::setMaximumSize(const float width, const float height) {
        maxWidth  = width;
        maxHeight = height;
    }
}
