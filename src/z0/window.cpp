/*
 * Copyright (c) 2024 Henri Michelon
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
#include <time.h>
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

    // Rendering Window proc
    LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
        static float lastMouseX = -1.0f;
        static float lastMouseY = -1.0f;
        auto* window = reinterpret_cast<z0::Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        auto& app = Application::get();
        switch (message) {
            case WM_CREATE:
                // Save the Window pointer passed to CreateWindowEx
                SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
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
                    app._stop(true);
                    ShowWindow(hwnd, SW_MINIMIZE);
                } else if (wParam == SC_RESTORE ) {
                    ShowWindow(hwnd, SW_RESTORE);
                    app._stop(false);
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
                    app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                app._onInput(event);
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
                if ((lastMouseX != -1) && (lastMouseY != -1)) {
                    auto dx = xPos - lastMouseX;
                    auto dy = yPos - lastMouseY;
                    auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, dx, dy);
                    app._onInput(event);
                } else {
                    auto event = InputEventMouseMotion(_getMouseButtonState(wParam), _getKeyboardModifiers(), xPos, yPos, 0, 0);
                    app._onInput(event);
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
                return TRUE;
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
    #ifndef DISABLE_LOG
            _mainThreadId = GetCurrentThreadId();
            if (Application::get().getConfig().loggingMode & LOGGING_MODE_WINDOW) {
                createLogWindow(hInstance);
            }
            if (Application::get().getConfig().loggingMode & LOGGING_MODE_FILE) {
                _logFile = fopen("log.txt", "w");
                if(_logFile == nullptr) {
                    die("Error opening log file");
                }
            }
            if (Application::get().getConfig().loggingMode != LOGGING_MODE_NONE) {
                log("===== START OF LOG =====");
            }
    #endif

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
            if (Application::get().getConfig().windowMonitor < monitorData.enumIndex) {
                monitorData.monitorIndex = Application::get().getConfig().windowMonitor;
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
    #ifndef DISABLE_LOG
            if (Application::get().getConfig().loggingMode & LOGGING_MODE_WINDOW) {
                CloseWindow(_hwndLog);
            }
            if (Application::get().getConfig().loggingMode & LOGGING_MODE_FILE) {
                fclose(_logFile);
            }
    #endif
        }

    #endif

    #ifndef DISABLE_LOG
    #ifdef _WIN32



        void CopyTextToClipboard(const string& text) {
            if (OpenClipboard(nullptr)) {
                EmptyClipboard();

                const size_t size = (text.size() + 1) * sizeof(wchar_t);
                const auto hMem = GlobalAlloc(GMEM_MOVEABLE, size);
                if (hMem) {
                    memcpy(GlobalLock(hMem), text.c_str(), size);
                    GlobalUnlock(hMem);
                    SetClipboardData(CF_TEXT, hMem);
                }
                CloseClipboard();
                if (hMem) {
                    GlobalFree(hMem);
                }
            }
        }

        LRESULT CALLBACK WindowProcedureLog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
            static HFONT hFont;
            switch (message) {
                case WM_CREATE: {
                    Window::_hwndLogList = CreateWindow(
                        "LISTBOX",
                        nullptr,
                        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL |LBS_STANDARD,
                        0, 0, 0, 0,
                        hwnd,
                        nullptr,
                        GetModuleHandle(nullptr),
                        nullptr
                    );
                    hFont =  CreateFont(
                        14,                        // Height of font
                        0,                         // Width of font
                        0,                         // Angle of escapement
                        0,                         // Orientation angle
                        FW_NORMAL,                 // Font weight
                        FALSE,                     // Italic attribute option
                        FALSE,                     // Underline attribute option
                        FALSE,                     // Strikeout attribute option
                        DEFAULT_CHARSET,           // Character set identifier
                        OUT_DEFAULT_PRECIS,        // Output precision
                        CLIP_DEFAULT_PRECIS,       // Clipping precision
                        DEFAULT_QUALITY,           // Output quality
                        DEFAULT_PITCH | FF_SWISS,  // Pitch and family
                        TEXT("Consolas"));            // Typeface name
                    SendMessage(Window::_hwndLogList, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
                    return 0;
                }
                case WM_COMMAND: {
                    auto subcommande = HIWORD(wParam);
                    if (subcommande == LBN_SELCHANGE) {
                        const int index = SendMessage(Window::_hwndLogList, LB_GETCURSEL, 0, 0);
                        if (index != LB_ERR) {
                            const auto length = SendMessage(Window::_hwndLogList, LB_GETTEXTLEN, index, 0);
                            auto buffer = new TCHAR[length+1];
                            SendMessage(Window::_hwndLogList, LB_GETTEXT, index, reinterpret_cast<LPARAM>(buffer));
                            buffer[length] = 0;
                            CopyTextToClipboard(buffer);
                            delete[] buffer;
                        }
                    }
                    break;
                }
                case WM_SIZE: {
                    if (Window::_hwndLogList) {
                        const int width = LOWORD(lParam);
                        const int height = HIWORD(lParam);
                        MoveWindow(Window::_hwndLogList, 0, 0, width, height, TRUE);
                    }
                    return 0;
                }
                case WM_DESTROY: {
                    DeleteObject(hFont);
                }
                default: break;
            }
            return DefWindowProc(hwnd, message, wParam, lParam);
        }

        HWND Window::_hwndLog{nullptr};
        HWND Window::_hwndLogList{nullptr};
        constexpr char szClassNameLog[ ] = "WindowsAppLog";
        list<string> Window::_deferredLogMessages;
        DWORD Window::_mainThreadId;
        FILE* Window::_logFile;

        void Window::createLogWindow(const HMODULE hInstance) {
            auto monitorEnumData = MonitorEnumData {
                .monitorIndex = Application::get().getConfig().loggingMonitor
            };
            EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorEnumData));

            const WNDCLASSEX wincl {
                .cbSize = sizeof(WNDCLASSEX),
                .style = CS_DBLCLKS,
                .lpfnWndProc = WindowProcedureLog,
                .cbClsExtra = 0,
                .cbWndExtra = 0,
                .hInstance = hInstance,
                .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
                .hCursor = LoadCursor(nullptr, IDC_ARROW),
                .hbrBackground = background,
                .lpszMenuName = nullptr,
                .lpszClassName = szClassNameLog,
                .hIconSm = LoadIcon(nullptr, IDI_APPLICATION),
            };
            if (!RegisterClassEx(&wincl)) die("Cannot register Log Window class");

            _hwndLog = CreateWindowEx(
                    WS_EX_OVERLAPPEDWINDOW,
                    szClassNameLog,
                    "Log",
                    WS_OVERLAPPEDWINDOW,
                    monitorEnumData.monitorRect.left,
                    monitorEnumData.monitorRect.top,
                    800,
                    600,
                    HWND_DESKTOP,
                    nullptr,
                    hInstance,
                    this
            );
            if (_hwndLog == nullptr) { die("Cannot create Log Window", std::to_string(GetLastError())); };
            ShowWindow(_hwndLog, SW_SHOW);
            UpdateWindow(_hwndLog);
        }

        void Window::_log(string msg) {
            const auto logMode = Application::get().getConfig().loggingMode;
            if (logMode == LOGGING_MODE_NONE) { return; }
            using namespace chrono;
            const auto in_time_t = system_clock::to_time_t(system_clock::now());
            tm tm;
            localtime_s(&tm, &in_time_t);
            string item = wstring_to_string(format(L"{:02}:{:02}:{:02}", tm.tm_hour, tm.tm_min, tm.tm_sec));
            item.append(" ");
            item.append(msg);
            if (static_cast<int>(logMode) & static_cast<int>(LOGGING_MODE_WINDOW)) {
                // Store the log message in the deferred log queue if we log from another thread
                // because the log Window proc run on the main thread
                if ((_hwndLogList != nullptr) && (GetCurrentThreadId() == _mainThreadId)) {
                    SendMessage(_hwndLogList, LB_INSERTSTRING, -1, reinterpret_cast<LPARAM>(item.c_str()));
                    const int itemCount = SendMessage(_hwndLogList, LB_GETCOUNT, 0, 0);
                    SendMessage(_hwndLogList, LB_SETTOPINDEX, itemCount-1, 0);
                } else {
                    _deferredLogMessages.push_back(item);
                }
            }
            if (logMode & LOGGING_MODE_STDOUT) {
                cout << item << endl;
            }
            if (logMode & LOGGING_MODE_FILE) {
                fwrite(item.c_str(), item.size(), 1, _logFile);
                fwrite("\n", 1, 1, _logFile);
            }
        }

        void Window::_processDeferredLog() {
            if (Application::get().getConfig().loggingMode & LOGGING_MODE_WINDOW) {
                for(const auto& msg: _deferredLogMessages) {
                    SendMessage(_hwndLogList, LB_INSERTSTRING, -1, reinterpret_cast<LPARAM>(msg.c_str()));
                    const int itemCount = SendMessage(_hwndLogList, LB_GETCOUNT, 0, 0);
                    SendMessage(_hwndLogList, LB_SETTOPINDEX, itemCount-1, 0);
                }
                _deferredLogMessages.clear();
            }
        }
    #endif
#endif


}
