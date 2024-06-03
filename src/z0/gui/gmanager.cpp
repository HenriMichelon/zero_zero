#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwindow.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/renderers/vector_renderer.h"
#include "z0/gui/gmanager.h"
#endif

namespace z0 {

    GManager::GManager(shared_ptr<VectorRenderer> &renderer,
                       const string& defaultFontName,
                       uint32_t defaultFontSize):
        vectorRenderer{renderer} {
        defaultFont = make_shared<Font>(defaultFontName, defaultFontSize);
    }

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
#ifdef _WIN32
            CURSORINFO ci {
                .cbSize = sizeof(CURSORINFO)
            };
            if (GetCursorInfo(&ci)) {
                // Mouse cursor is hidden
                if (ci.flags == 0) {
                    return false;
                }
            }
#endif
            auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(inputEvent);
            auto scaleX = vectorRenderer->SCALE.x / static_cast<float>(vectorRenderer->getDevice().getSwapChainExtent().width);
            auto scaleY = vectorRenderer->SCALE.y / static_cast<float>(vectorRenderer->getDevice().getSwapChainExtent().height);
            auto x = static_cast<int32_t>(mouseInputEvent.getX() * scaleX);
            auto y = static_cast<int32_t>(mouseInputEvent.getY() * scaleY);
            for (auto& window: windows) {
                auto consumed = false;
                auto lx = x - window->getRect().x;
                auto ly = y - window->getRect().y;
                if (mouseInputEvent.isPressed()) {
                    if (window->getRect().contains(x, y)) {
                        consumed |= window->eventMouseDown(mouseInputEvent.getMouseButton(), lx, ly);
                    }
                } else {
                    consumed |= window->eventMouseUp(mouseInputEvent.getMouseButton(), lx, ly);
                }
                if (consumed) { return true; }
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