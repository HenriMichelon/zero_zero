#include "z0/gui/gmanager.h"

namespace z0 {

    GManager::GManager(shared_ptr<VectorRenderer> &renderer): vectorRenderer{renderer} {}

    GManager::~GManager() {
        for (auto& window: windows) {
            window->eventDestroy();
        }
        windows.clear();
    }

    void GManager::drawFrame() {
        if (!needRedraw) { return; }
        needRedraw = false;
        vectorRenderer->beginDraw();
        for (auto& window: windows) {
            window->draw();
        }
        vectorRenderer->endDraw();
    }

    void GManager::add(const shared_ptr<GWindow> &window) {
        windows.push_back(window);
        window->windowManager = this;
        window->eventCreate();
        if (window->isVisible()) { window->eventShow(); }
        needRedraw = true;
    }

    bool GManager::onInput(InputEvent &inputEvent) {
        if (inputEvent.getType() == INPUT_EVENT_KEY) {
            auto &keyInputEvent = dynamic_cast<InputEventKey &>(inputEvent);
            if ((focusedWindow != nullptr) && (focusedWindow->isVisible())) {
                if (keyInputEvent.isPressed()) {
                    return focusedWindow->eventKeybDown(keyInputEvent.getKeyCode());
                } else {
                    return focusedWindow->eventKeybUp(keyInputEvent.getKeyCode());
                }
            }
        }
        return false;
    }

    void GManager::windowHidden(GWindow *window) {
        if (focusedWindow == window) {
            window->eventLostFocus();
            if (windows.empty()) {
                focusedWindow = nullptr;
            } else {
                focusedWindow = windows.back().get();
                focusedWindow->eventGotFocus();
            }
        }
    }

    void GManager::windowShown(GWindow *window) {
        if (focusedWindow) { focusedWindow->eventLostFocus(); }
        focusedWindow = window;
        window->eventGotFocus();
    }

}