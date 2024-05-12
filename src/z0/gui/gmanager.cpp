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
        } else if (inputEvent.getType() == INPUT_EVENT_MOUSE_BUTTON) {
            auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(inputEvent);
            auto scaleX = vectorRenderer->SCALE.x / static_cast<float>(vectorRenderer->getDevice().getSwapChainExtent().width);
            auto scaleY = vectorRenderer->SCALE.y / static_cast<float>(vectorRenderer->getDevice().getSwapChainExtent().height);
            auto x = static_cast<uint32_t>(mouseInputEvent.getX() * scaleX);
            auto y = static_cast<uint32_t>(mouseInputEvent.getY() * scaleY);
            for (auto& window: windows) {
                if (window->getRect().contains(x, y)) {
                    x = x - window->getRect().x;
                    y = y - window->getRect().y;
                    if (mouseInputEvent.isPressed()) {
                        return window->eventMouseDown(mouseInputEvent.getMouseButton(), x, y);
                    } else {
                        return window->eventMouseUp(mouseInputEvent.getMouseButton(), x, y);
                    }
                }
            }
        } else if (inputEvent.getType() == INPUT_EVENT_MOUSE_MOTION) {
            //die("Not implemented");
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