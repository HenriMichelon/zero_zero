/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cassert>
#include "z0/libraries.h"

module z0.ui.Manager;

import z0.Application;
import z0.Constants;
import z0.resources.Font;
import z0.Input;
import z0.InputEvent;

import z0.ui.Rect;
import z0.ui.Window;

import z0.vulkan.VectorRenderer;

namespace z0 {

    namespace ui {
        Manager::Manager(shared_ptr<VectorRenderer> &renderer,
                           const string& defaultFontName,
                           const uint32_t defaultFontSize):
            vectorRenderer{renderer} {
            defaultFont = make_shared<Font>(defaultFontName, defaultFontSize);
        }

        Manager::~Manager() {
            for (const auto& window: windows) {
                window->eventDestroy();
            }
            windows.clear();
        }

        void Manager::drawFrame() {
            auto lock = lock_guard(windowsMutex);
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
            if (vectorRenderer) {
                vectorRenderer->beginDraw();
                for (const auto& window: windows) {
                    window->draw();
                }
                vectorRenderer->endDraw();
            }
        }

        shared_ptr<ui::Window> Manager::add(const shared_ptr<Window> &window) {
            assert(window->windowManager == nullptr);
            {
                auto lock = lock_guard(windowsMutex);
                windows.push_back(window);
                window->windowManager = this;
            }
            window->eventCreate();
            if (window->isVisible()) { window->eventShow(); }
            needRedraw = true;
            return window;
        }

        void Manager::remove(const shared_ptr<Window>&window) {
            assert(window->windowManager != nullptr);
            removedWindows.push_back(window);
        }

        bool Manager::onInput(InputEvent &inputEvent) {
            if (inputEvent.getType() == InputEventType::KEY) {
                const auto &keyInputEvent = dynamic_cast<InputEventKey &>(inputEvent);
                if ((focusedWindow != nullptr) && (focusedWindow->isVisible())) {
                    if (keyInputEvent.isPressed()) {
                        return focusedWindow->eventKeybDown(keyInputEvent.getKey());
                    } else {
                        return focusedWindow->eventKeybUp(keyInputEvent.getKey());
                    }
                }
            } else if ((inputEvent.getType() == InputEventType::MOUSE_BUTTON)
                    || (inputEvent.getType() == InputEventType::MOUSE_MOTION)) {
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
                const auto& wnd = app().getWindow();
                const auto scaleX = app().getVectorExtent().x / static_cast<float>(wnd.getWidth());
                const auto scaleY = app().getVectorExtent().y / static_cast<float>(wnd.getHeight());
                const auto x = mouseEvent.getX() * scaleX;
                const auto y = mouseEvent.getY() * scaleY;

                if (inputEvent.getType() == InputEventType::MOUSE_MOTION) {
                    const auto resizeDeltaY = scaleY * resizeDelta;
                    if (resizedWindow != nullptr) {
                        if (resizingWindow) {
                            Rect rect = resizedWindow->getRect();
                            if (currentCursor == MouseCursor::RESIZE_H) {
                                const auto lx = x - rect.x;
                                if (resizingWindowOriginBorder) {
                                    rect.width = rect.width - lx;
                                    rect.x = x;
                                } else {
                                    rect.width = lx;
                                }
                            } else {
                                const auto ly = y - rect.y;
                                if (resizingWindowOriginBorder) {
                                    rect.height = rect.height - ly;
                                    rect.y = y;
                                } else {
                                    rect.height = ly;
                                }
                            }
                            resizedWindow->setRect(rect);
                            Input::setMouseCursor(currentCursor);
                            return true;
                        }
                        currentCursor = MouseCursor::ARROW;
                        resizedWindow = nullptr;
                        Input::setMouseCursor(currentCursor);
                    }
                    for (const auto& window: windows) {
                        auto consumed = false;
                        const auto lx = ceil(x - window->getRect().x);
                        const auto ly = ceil(y - window->getRect().y);
                        if (window->getRect().contains(x, y)) {
                            if (enableWindowResizing && window->getWidget().isDrawBackground()) {
                                if ((window->getResizeableBorders() & Window::RESIZEABLE_RIGHT) &&
                                    (lx >= (window->getRect().width - resizeDelta))) {
                                    currentCursor = MouseCursor::RESIZE_H;
                                    resizedWindow = window;
                                    resizingWindowOriginBorder = false;
                                    } else if ((window->getResizeableBorders() & Window::RESIZEABLE_LEFT) &&
                                               (lx < resizeDelta)) {
                                        currentCursor = MouseCursor::RESIZE_H;
                                        resizedWindow = window;
                                        resizingWindowOriginBorder = true;
                                               } else if ((window->getResizeableBorders() & Window::RESIZEABLE_TOP) &&
                                                          (ly >= (window->getRect().height - resizeDeltaY))) {
                                                   currentCursor = MouseCursor::RESIZE_V;
                                                   resizedWindow = window;
                                                   resizingWindowOriginBorder = false;
                                                          } else if ((window->getResizeableBorders() & Window::RESIZEABLE_BOTTOM) &&
                                                                     (ly < resizeDeltaY)) {
                                                              currentCursor = MouseCursor::RESIZE_V;
                                                              resizedWindow = window;
                                                              resizingWindowOriginBorder = true;
                                                                     }
                            }
                            if (resizedWindow != nullptr) {
                                Input::setMouseCursor(currentCursor);
                                return true;
                            }
                            consumed |= window->eventMouseMove(mouseEvent.getButtonsState(), lx, ly);
                        }
                        if (consumed) { return true; }
                    }
                } else {
                    const auto &mouseInputEvent = dynamic_cast<InputEventMouseButton&>(mouseEvent);
                    if (resizedWindow != nullptr) {
                        if ((!resizingWindow) &&
                            (mouseInputEvent.getMouseButton() == MouseButton::LEFT) &&
                            (mouseInputEvent.isPressed())) {
                            resizingWindow = true;
                            } else if ((mouseInputEvent.getMouseButton() == MouseButton::LEFT) &&
                                       (!mouseInputEvent.isPressed())) {
                                currentCursor = MouseCursor::ARROW;
                                resizedWindow = nullptr;
                                resizingWindow = false;
                                       }
                        Input::setMouseCursor(currentCursor);
                        return true;
                    }
                    for (const auto& window: windows) {
                        auto consumed = false;
                        const auto lx = ceil(x - window->getRect().x);
                        const auto ly = ceil(y - window->getRect().y);
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

}