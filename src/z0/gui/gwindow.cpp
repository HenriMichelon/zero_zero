#include "z0/gui/gpanel.h"
#include "z0/gui/gmanager.h"

#include <ranges>

namespace z0 {

    GWindow::GWindow(Rect r): rect{r} {
        eventCreate();
    }

    GWindow::~GWindow() {
        eventDestroy();
    }

    void GWindow::draw(VectorRenderer&D) const {
        if (!isVisible()) return;
        D.setTranslate({rect.x, rect.y});
        widget->draw(D);
    }

    void GWindow::unFreeze(shared_ptr<GWidget>&W){
        for (auto& child: W->getChildren()) {
            unFreeze(child);
        }
        W->setFreezed(false);
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
        widget->layout = layout;
        widget->font = widget->layout->getFont();
        widget->layout->addResource(*widget, RES);
        widget->setDrawBackground(true);
        widget->eventCreate();
        widget->setPos(0, 0);
        widget->setSize(getWidth(), getHeight());
        focusedWidget = widget->setFocus();
        unFreeze(widget);
        return *widget;
    }

    void GWindow::setLayout(shared_ptr<GLayout> LAYOUT) {
        if (layout == nullptr) {
            layout = GLayout::create();
        } else {
            layout = std::move(LAYOUT);
        }
    }

    void GWindow::eventCreate() {
        onCreate();
        if (widget == nullptr) { setWidget(); }
        if (widget != nullptr ) { widget->resizeChildren(); }
    }

    void GWindow::eventDestroy() {
        if (widget) { widget->eventDestroy(); }
        onDestroy();
    }

    void GWindow::eventShow() {
        if (widget) { widget->eventShow(); }
        onShow();
        refresh();
    }

    void GWindow::eventKeybDown(Key K) {
/*	if ((K == keyb.KEY_TAB) && widget)
	{
//		widget->ClosePopup();
		if (focusedWidget)
			focusedWidget = focusedWidget->SetNextFocus();
		else
			focusedWidget = widget->SetFocus();
	}
	else*/ if (focusedWidget != nullptr) {
            focusedWidget->eventKeybDown(K);
        }
        onKeybDown(K);
        refresh();
    }

    void GWindow::eventKeybUp(Key K) {
        if (focusedWidget != nullptr) focusedWidget->eventKeybUp(K);
        onKeybUp(K);
        refresh();
    }

    void GWindow::eventMouseDown(MouseButton B, int32_t X, int32_t Y) {
        if (widget) {
            shared_ptr<GWidget>newfocused = widget->eventMouseDown(B, X, Y);
            if (newfocused.get() != focusedWidget) {
                if (focusedWidget != nullptr) { focusedWidget->setFocus(false); }
                focusedWidget = newfocused.get();
            }
        }
        onMouseDown(B, X, Y);
        refresh();
    }

    void GWindow::eventMouseUp(MouseButton B, int32_t X, int32_t Y) {
        if (widget) {
            widget->eventMouseUp(B, X, Y);
        }
        onMouseUp(B, X, Y);
        refresh();
    }

    void GWindow::eventMouseMove(MouseButton B, int32_t X, int32_t Y) {
        if ((focusedWidget != nullptr) &&
            (focusedWidget->mouseMoveOnFocus)) {
            focusedWidget->eventMouseMove(B, X, Y);
        }
        else if (widget) {
            widget->eventMouseMove(B, X, Y);
        }
        onMouseMove(B, X, Y);
        refresh();
    }

    void GWindow::refresh() {
        if (freezed) { return; }
        if (widget) { widget->refresh(); }
        freezed = true;
        if (isVisible()) {
            if (widget != nullptr) widget->flushRefresh();
            windowManager->refresh();
        }
        freezed = false;
    }

    void GWindow::setFocusedWidget(const shared_ptr<GWidget>& W) {
        focusedWidget = W.get();
    }

    GWidget& GWindow::getWidget() {
        return *widget;
    };

    void GWindow::setHeight(uint32_t h) {
        rect.height = h;
        refresh();
    }

    void GWindow::setWidth(uint32_t w) {
        rect.width = w;
        refresh();
    }

    void GWindow::setPos(int32_t x, int32_t y) {
        rect.x = x;
        rect.y = y;
        refresh();
    }

    shared_ptr<GLayout> GWindow::getLayout() const {
        return layout;
    };

    void GWindow::eventHide() {
        onHide();
    }

    void GWindow::eventGotFocus() {
        onGotFocus();
    }

    void GWindow::eventLostFocus() {
        onLostFocus();
    }
}