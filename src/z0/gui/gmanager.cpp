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
        try {
            auto& keyInputEvent = dynamic_cast<InputEventKey&>(inputEvent);
            if (focusedWindow != nullptr) {
                if (keyInputEvent.isPressed()) {
                    focusedWindow->eventKeybDown(keyInputEvent.getKeyCode());
                } else {
                    focusedWindow->eventKeybUp(keyInputEvent.getKeyCode());
                }
            }
            return false;
        } catch (const std::bad_cast& e) {
            return false;
        }
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