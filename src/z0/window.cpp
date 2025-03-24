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
#include <windowsx.h>
#include <hidsdi.h>
#endif
#include "z0/libraries.h"

module z0.Window;

import z0.Application;
import z0.ApplicationConfig;
import z0.Constants;
import z0.Input;
import z0.InputEvent;
import z0.Tools;

namespace z0 {

#ifdef _WIN32

    constexpr char szClassName[ ] = "WindowsApp";

    // Helper to translate Windows get state to z0 key modifiers
    int _getKeyboardModifiers() {
        int modifiers = 0;
        if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= static_cast<int>(KeyModifier::SHIFT);
        if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= static_cast<int>(KeyModifier::CONTROL);
        if (GetKeyState(VK_MENU) & 0x8000) modifiers |= static_cast<int>(KeyModifier::ALT);
        return modifiers;
    }

    uint32_t _getMouseButtonState(WPARAM wParam) {
        uint32_t state{0};
        if (wParam & MK_LBUTTON) state += static_cast<int>(MouseButton::LEFT);
        if (wParam & MK_MBUTTON) state += static_cast<int>(MouseButton::MIDDLE);
        if (wParam & MK_RBUTTON) state += static_cast<int>(MouseButton::RIGHT);
        return state;
    }

    bool Window::resettingMousePosition = false;

    // Rendering Window proc
    LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        static float lastMouseX = -1.0f;
        static float lastMouseY = -1.0f;
        auto* window = reinterpret_cast<z0::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        switch (message) {
            case WM_CREATE:
                // Save the Window pointer passed to CreateWindowEx
                SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT *>(lParam)->lpCreateParams));
                return 0;
            case WM_SIZE: {
                if (!IsIconic(hwnd)) {
                    // Get Window content size
                    RECT rect = {};
                    GetClientRect(hwnd, &rect);
                    window->_setSize(rect.right - rect.left, rect.bottom - rect.top);
                }
                break;
            }
            case WM_SYSCOMMAND: {
                if (wParam == SC_CLOSE) {
                    window->close();
                    return 0;
                }
                if (wParam == SC_MINIMIZE) {
                    app()._stop(true);
                    ShowWindow(hwnd, SW_MINIMIZE);
                } else if (wParam == SC_RESTORE ) {
                    ShowWindow(hwnd, SW_RESTORE);
                    app()._stop(false);
                }
            }
            break;
            case WM_KEYDOWN:{
                auto scanCode = static_cast<OsKey>((lParam & 0x00FF0000) >> 16);
                auto key = Input::osKeyToKey(scanCode);
                Input::_keyJustPressedStates[key] = !Input::_keyPressedStates[key];
                Input::_keyPressedStates[key] = true;
                Input::_keyJustReleasedStates[key] = false;
                if (Input::_keyJustPressedStates[key]) {
                    auto event = InputEventKey{key, true, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
                    app()._onInput(event);
                }
                break;
            }
            case WM_KEYUP: {
                auto scanCode = static_cast<OsKey>((lParam & 0x00FF0000) >> 16);
                auto key = Input::osKeyToKey(scanCode);
                Input::_keyPressedStates[key] = false;
                Input::_keyJustPressedStates[key] = false;
                Input::_keyJustReleasedStates[key] = true;
                auto event = InputEventKey{key, false, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
                app()._onInput(event);
                break;
            }
            case WM_LBUTTONDOWN: {
                Input::_mouseButtonJustPressedStates[MouseButton::LEFT] = !Input::_mouseButtonPressedStates[MouseButton::LEFT];
                Input::_mouseButtonPressedStates[MouseButton::LEFT] = true;
                Input::_mouseButtonJustReleasedStates[MouseButton::LEFT] = false;
                auto event = InputEventMouseButton(MouseButton::LEFT,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_LBUTTONUP: {
                Input::_mouseButtonJustPressedStates[MouseButton::LEFT] = false;
                Input::_mouseButtonPressedStates[MouseButton::LEFT] = false;
                Input::_mouseButtonJustReleasedStates[MouseButton::LEFT] = false;
                auto event = InputEventMouseButton(MouseButton::LEFT,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_RBUTTONDOWN: {
                Input::_mouseButtonJustPressedStates[MouseButton::RIGHT] = !Input::_mouseButtonPressedStates[MouseButton::RIGHT];
                Input::_mouseButtonPressedStates[MouseButton::RIGHT] = true;
                Input::_mouseButtonJustReleasedStates[MouseButton::RIGHT] = false;
                auto event = InputEventMouseButton(MouseButton::RIGHT,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_RBUTTONUP: {
                Input::_mouseButtonJustPressedStates[MouseButton::RIGHT] = false;
                Input::_mouseButtonPressedStates[MouseButton::RIGHT] = false;
                Input::_mouseButtonJustReleasedStates[MouseButton::RIGHT] = false;
                auto event = InputEventMouseButton(MouseButton::RIGHT,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_MBUTTONDOWN: {
                Input::_mouseButtonJustPressedStates[MouseButton::MIDDLE] = !Input::_mouseButtonPressedStates[MouseButton::MIDDLE];
                Input::_mouseButtonPressedStates[MouseButton::MIDDLE] = true;
                Input::_mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::MIDDLE,
                                                       true,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_MBUTTONUP: {
                Input::_mouseButtonJustPressedStates[MouseButton::MIDDLE] = false;
                Input::_mouseButtonPressedStates[MouseButton::MIDDLE] = false;
                Input::_mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::MIDDLE,
                                                       false,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(wParam),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_MOUSEWHEEL: {
                Input::_mouseButtonJustPressedStates[MouseButton::MIDDLE] = false;
                Input::_mouseButtonPressedStates[MouseButton::MIDDLE] = false;
                Input::_mouseButtonJustReleasedStates[MouseButton::MIDDLE] = false;
                auto event = InputEventMouseButton(MouseButton::WHEEL,
                                                       GET_WHEEL_DELTA_WPARAM(wParam) < 0,
                                                       _getKeyboardModifiers(),
                                                       _getMouseButtonState(GET_KEYSTATE_WPARAM(wParam)),
                                                       static_cast<float>(GET_X_LPARAM(lParam)),
                                                       static_cast<float>(window->getHeight())-GET_Y_LPARAM(lParam));
                app()._onInput(event);
                break;
            }
            case WM_MOUSEMOVE: {
                auto xPos = static_cast<float>(GET_X_LPARAM(lParam));
                auto yPos = 0.0f;
                auto y = GET_Y_LPARAM(lParam);
                if (y < window->getHeight()) {
                    yPos = static_cast<float>(window->getHeight()-y);
                } else if (y < 0){
                    yPos = static_cast<float>(window->getHeight());
                }
                if (!Window::resettingMousePosition) {
                    if ((lastMouseX != -1) && (lastMouseY != -1)) {
                        auto dx = xPos - lastMouseX;
                        auto dy = yPos - lastMouseY;
                        auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, dx, dy);
                        app()._onInput(event);
                    } else {
                        auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, 0, 0);
                        app()._onInput(event);
                    }
                } else {
                    Window::resettingMousePosition = false;
                }
                lastMouseX = xPos;
                lastMouseY = yPos;
                break;
            }
            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
        }
        return 0;
    }
    #endif

        string Window::toString() const {
            stringstream s;
            s << "Window " << width << "x" << height;
            return s.str();
        }

    #ifdef _WIN32

        struct MonitorEnumData {
            int  enumIndex{0};
            int  monitorIndex{0};
            RECT monitorRect{0};
        };
        BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
            const auto data = reinterpret_cast<MonitorEnumData*>(dwData);
            if (data->enumIndex == data->monitorIndex) {
                data->monitorRect = *lprcMonitor;
                return FALSE;
            }
            data->enumIndex++;
            return TRUE;
        }

        void Window::_setSize(int w, int h) {
            width = w;
            height = h;
        }

        Window::Window(const ApplicationConfig& applicationConfig):
            width{0},
            height{0},
            background{CreateSolidBrush(RGB(
                applicationConfig.clearColor.r*255.0f,
                applicationConfig.clearColor.g*255.0f,
                applicationConfig.clearColor.b*255.0f))} {
            auto hInstance = GetModuleHandle(nullptr);

            const WNDCLASSEX wincl {
                .cbSize = sizeof(WNDCLASSEX),
                .style = CS_DBLCLKS,
                .lpfnWndProc = WindowProcedure,
                .cbClsExtra = 0,
                .cbWndExtra = 0,
                .hInstance = hInstance,
                .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
                .hCursor = LoadCursor(nullptr, IDC_ARROW),
                .hbrBackground = background,
                .lpszMenuName = nullptr,
                .lpszClassName = szClassName,
                .hIconSm = LoadIcon(nullptr, IDI_APPLICATION),
            };
            if (!RegisterClassEx(&wincl)) die("Cannot register Window class");

            // Count the numbers of connected monitors
            auto monitorData = MonitorEnumData {
                .monitorIndex = -1
            };
            EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorData));
            // Adjust the monitor selection
            if (applicationConfig.windowMonitor < monitorData.enumIndex) {
                monitorData.monitorIndex = applicationConfig.windowMonitor;
            } else {
                monitorData.monitorIndex = 0;
            }
            monitorData.enumIndex = 0;
            // Getting the dimensions of the selected screen
            EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorData));
            auto screenWidth = monitorData.monitorRect.right - monitorData.monitorRect.left;
            auto screenHeight = monitorData.monitorRect.bottom - monitorData.monitorRect.top;

            int x = CW_USEDEFAULT;
            int y = CW_USEDEFAULT;
            int w = CW_USEDEFAULT;
            int h = CW_USEDEFAULT;

            DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER;
            DWORD exStyle = 0;
            switch (applicationConfig.windowMode) {
                case WindowMode::WINDOWED:{
                    RECT rect{
                        .left = 0,
                        .top = 0,
                        .right = static_cast<int>(applicationConfig.windowWidth),  // Desired width of the client area
                        .bottom = static_cast<int>(applicationConfig.windowHeight),
                    };
                    AdjustWindowRect(&rect, style, FALSE); // Adjust the rect to include the frame
                    w = rect.right - rect.left;
                    h = rect.bottom - rect.top;
                    x = static_cast<int>((screenWidth - w) / 2) + monitorData.monitorRect.left;
                    y = static_cast<int>((screenHeight - h) / 2) + monitorData.monitorRect.top;
                    break;
                }
                case WindowMode::WINDOWED_MAXIMIZED:{
                    style |=  WS_MAXIMIZE;
                    x = monitorData.monitorRect.left;
                    y = monitorData.monitorRect.top;
                    break;
                }
                case WindowMode::FULLSCREEN: {
                    DEVMODE dmScreenSettings = {};
                    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
                    dmScreenSettings.dmPelsWidth = static_cast<int>(applicationConfig.windowWidth);
                    dmScreenSettings.dmPelsHeight =  static_cast<int>(applicationConfig.windowHeight);
                    dmScreenSettings.dmBitsPerPel = 32;
                    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
                    if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                        die( "Display mode change failed");
                    }
                    screenWidth = monitorData.monitorRect.right - monitorData.monitorRect.left;
                    screenHeight = monitorData.monitorRect.bottom - monitorData.monitorRect.top;
                }
                case WindowMode::WINDOWED_FULLSCREEN:{
                    exStyle = WS_EX_APPWINDOW;
                    style = WS_POPUP | WS_MAXIMIZE;
                    x = monitorData.monitorRect.left;
                    y = monitorData.monitorRect.top;
                    w = static_cast<int>(screenWidth);
                    h = static_cast<int>(screenHeight);
                    break;
                }
                default:
                    die("Unknown WindowMode");
            }

            rect.left = x;
            rect.top = y;
            rect.right = rect.left + w;
            rect.bottom = rect.top + h;
            hwnd = CreateWindowEx(
                    exStyle,
                    szClassName,
                    applicationConfig.appName.c_str(),
                    style,
                    x,
                     y,
                    w,
                    h,
                    HWND_DESKTOP,
                    nullptr,
                    hInstance,
                    this
            );
            if (hwnd == nullptr) { die("Cannot create Window", std::to_string(GetLastError())); };

            RAWINPUTDEVICE rid[1];
            rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
            rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
            rid[0].dwFlags = 0;
            rid[0].hwndTarget = 0;
            if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) {
                die("Failed to register raw input devices.", std::to_string(GetLastError()));
            }

            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
            UpdateWindow(hwnd);
        }

        Window::~Window() {
            DeleteObject(background);
        }

    #endif


}
