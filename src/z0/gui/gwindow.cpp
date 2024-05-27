#include "z0/gui/gpanel.h"
#include "z0/gui/gmanager.h"

#include <ranges>

namespace z0 {

    GWindow::GWindow(Rect r): rect{r} {
        eventCreate();
    }

    void GWindow::draw() const {
        if (!isVisible()) return;
        windowManager->getRenderer().setTranslate({rect.x, rect.y});
        windowManager->getRenderer().setTransparency(1.0f - transparency);
        widget->draw(windowManager->getRenderer());
    }

    void GWindow::unFreeze(shared_ptr<GWidget>&W){
        for (auto& child: W->getChildren()) {
            unFreeze(child);
        }
        W->setFreezed(false);
    }

    shared_ptr<Font> &GWindow::getDefaultFont() const {
        return windowManager->defaultFont;
    }

    GWidget& GWindow::setWidget(shared_ptr<GWidget>WIDGET, const string&RES, int32_t PADDING){
        if (layout == nullptr) { setLayout(nullptr); }
        if (WIDGET == nullptr) {
            widget = make_shared<GPanel>();
        }
        else {
            widget = std::move(WIDGET);
        }
        widget->freeze = true;
        widget->padding = PADDING;
        widget->window = this;
        widget->style = layout;
        widget->font = widget->style->getFont();
        widget->style->addResource(*widget, RES);
        widget->setDrawBackground(true);
        widget->eventCreate();
        widget->setPos(0, 0);
        widget->setSize(getWidth(), getHeight());
        focusedWidget = widget->setFocus();
        unFreeze(widget);
        return *widget;
    }

    void GWindow::setLayout(shared_ptr<GStyle> LAYOUT) {
        if (layout == nullptr) {
            layout = GStyle::create();
        } else {
            layout = std::move(LAYOUT);
        }
        refresh();
    }

    void GWindow::setVisible(bool isVisible) {
        if (visible != isVisible) {
            visible = isVisible;
            if (visible) {
                eventShow();
            } else {
                eventHide();
            }
        }
    }

    void GWindow::hide() {
        setVisible(false);
    }

    void GWindow::show() {
        setVisible(true);
    }

    void GWindow::eventCreate() {
        onCreate();
        if (widget == nullptr) { setWidget(); }
        if (widget != nullptr ) { widget->resizeChildren(); }
    }

    void GWindow::eventDestroy() {
        if (widget) { widget->eventDestroy(); }
        onDestroy();
        widget.reset();
    }

    void GWindow::eventShow() {
        if (windowManager) { windowManager->windowShown(this); }
        if (widget) { widget->eventShow(); }
        onShow();
        refresh();
    }

    bool GWindow::eventKeybDown(Key K) {
        bool consumed = false;
	    if (focusedWidget) {
            consumed = focusedWidget->eventKeybDown(K); // XXX consumed
        }
        consumed |= onKeyDown(K);
        refresh();
        return consumed;
    }

    bool GWindow::eventKeybUp(Key K) {
        bool consumed = false;
        if (focusedWidget) {
            focusedWidget->eventKeybUp(K); // XXX consumed
        }
        consumed |= onKeyUp(K);
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseDown(MouseButton B, int32_t X, int32_t Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) { consumed = widget->eventMouseDown(B, X, Y); }
        consumed |= onMouseDown(B, X, Y);
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseUp(MouseButton B, int32_t X, int32_t Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if (widget) { consumed = widget->eventMouseUp(B, X, Y); }
        consumed |= onMouseUp(B, X, Y);
        refresh();
        return consumed;
    }

    bool GWindow::eventMouseMove(MouseButton B, int32_t X, int32_t Y) {
        if (!visible) { return false; }
        bool consumed = false;
        if ((focusedWidget != nullptr) &&
            (focusedWidget->mouseMoveOnFocus)) {
            consumed = focusedWidget->eventMouseMove(B, X, Y);
        }
        else if (widget) {
            consumed = widget->eventMouseMove(B, X, Y);
        }
        consumed |= onMouseMove(B, X, Y);
        refresh();
        return consumed;
    }

    void GWindow::refresh() {
        if (windowManager) windowManager->refresh();
    }

    void GWindow::setFocusedWidget(const shared_ptr<GWidget>& W) {
        focusedWidget = W.get();
    }

    GWidget& GWindow::getWidget() {
        return *widget;
    }

    void GWindow::setHeight(int32_t h) {
        rect.height = h;
        eventResize();
    }

    void GWindow::setWidth(int32_t w) {
        rect.width = w;
        eventResize();
    }

    void GWindow::setPos(int32_t x, int32_t y) {
        rect.x = x;
        rect.y = y;
        eventMove();
    }

    shared_ptr<GStyle> GWindow::getLayout() const {
        return layout;
    }

    void GWindow::setTransparency(float alpha) {
        transparency = alpha;
        refresh();
    }

    void GWindow::eventResize() {
        if (widget) { widget->setSize(rect.width, rect.height); }
        onResize();
        refresh();
    }

    void GWindow::eventMove() {
        onMove();
        refresh();
    }

    void GWindow::eventHide() {
        if (windowManager) { windowManager->windowHidden(this); }
        onHide();
        refresh();
    }

    void GWindow::eventGotFocus() {
        onGotFocus();
        refresh();
    }

    void GWindow::eventLostFocus() {
        onLostFocus();
        refresh();
    }
}