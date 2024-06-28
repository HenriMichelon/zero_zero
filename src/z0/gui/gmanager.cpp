#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/resources/image.h"
#include "z0/resources/font.h"
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/input.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwidget.h"
#include "z0/gui/gwindow.h"
#include "z0/renderers/renderpass.h"
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
        for(const auto&window : removedWindows) {
            window->windowManager = nullptr;
            if (window->isVisible()) { window->eventHide(); }
            window->eventDestroy();
            windows.remove(window);
            needRedraw = true;
        }
        removedWindows.clear();
        for (auto& window: windows) {
            if (window->visibilityChanged) {
                window->visibilityChanged = false;
                window->visible = window->visibilityChange;
                needRedraw = true;
                if (window->visible) {
                    if (focusedWindow) { focusedWindow->eventLostFocus(); }
                    focusedWindow = window;
                    window->eventGotFocus();
                    window->eventShow();
                } else {
                    if (focusedWindow == window) {
                        window->eventLostFocus();
                        if (windows.empty()) {
                            focusedWindow = nullptr;
                        } else {
                            focusedWindow = windows.back();
                            focusedWindow->eventGotFocus();
                        }
                    }
                    window->eventHide();
                }
            }
        }
        if (!needRedraw) { return; }
        needRedraw = false;
        vectorRenderer->beginDraw();
        for (auto& window: windows) {
            window->draw();
        }
        vectorRenderer->endDraw();
    }

    void GManager::add(const shared_ptr<GWindow> &window) {
        assert(window->windowManager == nullptr);
        windows.push_back(window);
        window->windowManager = this;
        window->eventCreate();
        if (window->isVisible()) { window->eventShow(); }
        needRedraw = true;
    }

    void GManager::remove(const shared_ptr<GWindow>&window) {
        assert(window->windowManager != nullptr);
        removedWindows.push_back(window);
    }

    bool GManager::onInput(InputEvent &inputEvent) {
        if (inputEvent.getType() == INPUT_EVENT_KEY) {
            auto &keyInputEvent = dynamic_cast<InputEventKey &>(inputEvent);
            if ((focusedWindow != nullptr) && (focusedWindow->isVisible())) {
                if (keyInputEvent.isPressed()) {
                    return focusedWindow->eventKeybDown(keyInputEvent.getKey());
                } else {
                    return focusedWindow->eventKeybUp(keyInputEvent.getKey());
                }
            }
        } else if ((inputEvent.getType() == INPUT_EVENT_MOUSE_BUTTON) 
                || (inputEvent.getType() == INPUT_EVENT_MOUSE_MOTION)) {
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
            auto &mouseEvent = dynamic_cast<InputEventMouse&>(inputEvent);
            const auto& wnd = Application::get().getWindow();
            auto scaleX = VECTOR_SCALE.x / static_cast<float>(wnd.getWidth());
            auto scaleY = VECTOR_SCALE.y / static_cast<float>(wnd.getHeight());
            auto x = mouseEvent.getX() * scaleX;
            auto y = mouseEvent.getY() * scaleY;

            if (inputEvent.getType() == INPUT_EVENT_MOUSE_MOTION) {
                if (resizingWindow) {
                    Input::setMouseCursor(currentCursor);
                    Rect rect = resizedWindow->getRect();
                    if (currentCursor == MOUSE_CURSOR_RESIZE_H) {
                        auto lx = x - rect.x;
                        if (resizingWindowOriginBorder) {
                            rect.width = rect.width - lx;
                            rect.x = x;
                        } else {
                            rect.width = lx;
                        }
                    } else {
                        auto ly = y - rect.y;
                        if (resizingWindowOriginBorder) {
                            rect.height = rect.height - ly;
                            rect.y = y;
                        } else {
                            rect.height = ly;
                        }
                    }
                    if ((rect.width < (resizeDelta + resizedWindow->getMinimumWidth())) || 
                        (rect.height < (resizeDelta + resizedWindow->getMinimumHeight())) ||
                        (rect.width > resizedWindow->getMaximumWidth()) ||
                        (rect.height > resizedWindow->getMaximumHeight())) {
                        return true;
                    }
                    resizedWindow->setRect(rect);
                    return true;
                }
                for (auto& window: windows) {
                    auto consumed = false;
                    auto lx = x - window->getRect().x;
                    auto ly = y - window->getRect().y;
                    if (window->getRect().contains(x, y)) {
                        if (window->getWidget().isDrawBackground()) {
                            if ((window->getResizeableBorders() & GWindow::RESIZEABLE_RIGHT) && (lx > (window->getRect().width - resizeDelta))) {
                                currentCursor = MOUSE_CURSOR_RESIZE_H;
                                resizedWindow = window;
                                resizingWindowOriginBorder = false;
                            } else if ((window->getResizeableBorders() & GWindow::RESIZEABLE_LEFT) && (lx < resizeDelta)) {
                                currentCursor = MOUSE_CURSOR_RESIZE_H;
                                resizedWindow = window;
                                resizingWindowOriginBorder = true;
                            } else if ((window->getResizeableBorders() & GWindow::RESIZEABLE_TOP) && (ly > (window->getRect().height - resizeDelta))) {
                                currentCursor = MOUSE_CURSOR_RESIZE_V;
                                resizedWindow = window;
                                resizingWindowOriginBorder = false;
                            } else if ((window->getResizeableBorders() & GWindow::RESIZEABLE_BOTTOM) && (ly < resizeDelta)) {
                                currentCursor = MOUSE_CURSOR_RESIZE_V;
                                resizedWindow = window;
                                resizingWindowOriginBorder = true;
                            } else if (resizedWindow != nullptr) {
                                currentCursor = MOUSE_CURSOR_ARROW;
                                resizedWindow = nullptr;
                                Input::setMouseCursor(currentCursor);
                            }
                        }
                        if (resizedWindow != nullptr) {
                            Input::setMouseCursor(currentCursor);
                            consumed = true;
                        } else {
                            consumed |= window->eventMouseMove(mouseEvent.getButtonsState(), lx, ly);
                        }
                    }
                    if (consumed) { return true; }
                }
            } else {
                auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(mouseEvent);
                if (resizedWindow != nullptr) {
                    if ((!resizingWindow) &&
                        (mouseInputEvent.getMouseButton() == MOUSE_BUTTON_LEFT) && 
                        (mouseInputEvent.isPressed())) {
                        resizingWindow = true;
                    } else if ((mouseInputEvent.getMouseButton() == MOUSE_BUTTON_LEFT) &&
                               (!mouseInputEvent.isPressed())) {
                        currentCursor = MOUSE_CURSOR_ARROW;
                        resizedWindow = nullptr;
                        resizingWindow = false;
                    }
                    Input::setMouseCursor(currentCursor);
                    return true;
                }
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
            }
        }
        return false;
    }

}