#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/nodes/node.h"
#include "z0/application.h"
#include "z0/input.h"
#endif

#ifdef _WIN32

const char szClassName[ ] = "WindowsApp";

// Helper to translate Windows get state to z0 key modifiers
int _getKeyboardModifiers() {
    int modifiers = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= z0::KEY_MODIFIER_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= z0::KEY_MODIFIER_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000) modifiers |= z0::KEY_MODIFIER_ALT;
    return modifiers;
}

// Rendering window proc
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static float lastMouseX = -1.0f;
    static float lastMouseY = -1.0f;
    auto* window = (z0::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto& app = z0::Application::get();
    switch (message) {
        case WM_CREATE:
            // Save the Window pointer passed to CreateWindowEx
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
            return 0;
        case WM_SIZE: {
            if (!IsIconic(hwnd)) {
                // Get window content size
                RECT rect = {};
                GetClientRect(hwnd, &rect);
                window->_setSize(rect.right - rect.left, rect.bottom - rect.top);
            }
            break;
        }
        case WM_SYSCOMMAND:
            if (wParam == SC_CLOSE) {
                window->close();
                return 0;
            } else if (wParam == SC_MINIMIZE) {
                app._stop(true);
                ShowWindow(hwnd, SW_MINIMIZE);
            } else if (wParam == SC_RESTORE ) {
                ShowWindow(hwnd, SW_RESTORE);
                app._stop(false);
            }
            break;
        case WM_KEYDOWN: {
            auto scanCode = static_cast<z0::OsKey>((lParam & 0x00FF0000) >> 16);
            auto key = z0::Input::osKeyToKey(scanCode);
            z0::Input::_keyJustPressedStates[key] = !z0::Input::_keyPressedStates[key];
            z0::Input::_keyPressedStates[key] = true;
            z0::Input::_keyJustReleasedStates[key] = false;
            auto event = z0::InputEventKey{key, true, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
            app._onInput(event);
            break;
        }
        case WM_KEYUP: {
            auto scanCode = static_cast<z0::OsKey>((lParam & 0x00FF0000) >> 16);
            auto key = z0::Input::osKeyToKey(scanCode);
            z0::Input::_keyPressedStates[key] = false;
            z0::Input::_keyJustPressedStates[key] = false;
            z0::Input::_keyJustReleasedStates[key] = true;
            auto event = z0::InputEventKey{key, false, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
            app._onInput(event);
            break;
        }
        case WM_LBUTTONDOWN: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_LEFT] = !z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_LEFT];
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_LEFT] = true;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_LEFT] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_LEFT,
                                                   true,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
            app._onInput(event);
            break;
        }
        case WM_LBUTTONUP: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_LEFT] = false;
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_LEFT] = false;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_LEFT] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_LEFT,
                                                   false,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
            app._onInput(event);
            break;
        }
        case WM_RBUTTONDOWN: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_RIGHT] = !z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_RIGHT];
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_RIGHT] = true;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_RIGHT] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_RIGHT,
                                                   true,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
            app._onInput(event);
            break;
        }
        case WM_RBUTTONUP: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_RIGHT] = false;
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_RIGHT] = false;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_RIGHT] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_RIGHT,
                                                   false,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
            app._onInput(event);
            break;
        }
        case WM_MBUTTONDOWN: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_MIDDLE] = !z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_MIDDLE];
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_MIDDLE] = true;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_MIDDLE] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_MIDDLE,
                                                   true,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
            app._onInput(event);
            break;
        }
        case WM_MBUTTONUP: {
            z0::Input::_mouseButtonJustPressedStates[z0::MOUSE_BUTTON_MIDDLE] = false;
            z0::Input::_mouseButtonPressedStates[z0::MOUSE_BUTTON_MIDDLE] = false;
            z0::Input::_mouseButtonJustReleasedStates[z0::MOUSE_BUTTON_MIDDLE] = false;
            auto event = z0::InputEventMouseButton(z0::MOUSE_BUTTON_MIDDLE,
                                                   false,
                                                   _getKeyboardModifiers(),
                                                   LOWORD(lParam),
                                                   static_cast<float>(window->getHeight())-HIWORD(lParam));
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
                auto event = z0::InputEventMouseMotion(xPos, yPos, dx, dy);
                app._onInput(event);
            } else {
                auto event = z0::InputEventMouseMotion(xPos, yPos, 0, 0);
                app._onInput(event);
            }
            lastMouseX = xPos;
            lastMouseY = yPos;
            break;
        }
        case WM_INPUT: {
            UINT dwSize;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                            RID_INPUT,
                            nullptr,
                            &dwSize,
                            sizeof(RAWINPUTHEADER));
            auto lpb = new BYTE[dwSize];
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                                RID_INPUT,
                                lpb,
                                &dwSize,
                                sizeof(RAWINPUTHEADER)) != dwSize) {
                z0::die("GetRawInputData does not return correct size");
            }

            auto* raw = reinterpret_cast<RAWINPUT*>(lpb);
            if (raw->header.dwType == RIM_TYPEKEYBOARD) {
                // for Input::getKeyboardVector
                const RAWKEYBOARD& rawKB = raw->data.keyboard;
                bool keyDown = !(rawKB.Flags & RI_KEY_BREAK);
                UINT key = rawKB.MakeCode;
                if (key < 256) {
                    z0::Input::_keys[key] = keyDown;
                }
            }
            delete[] lpb;
            break;
        }
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
#endif

namespace z0 {

    string Window::toString() const {
        stringstream s;
        s << "Window " << width << "x" << height;
        return s.str();
    }

#ifdef _WIN32

    void Window::_setSize(int w, int h) {
        width = w;
        height = h;
    }

    Window::Window(const ApplicationConfig& applicationConfig):
        width{0},
        height{0},
        background{CreateSolidBrush(RGB(z0::WINDOW_CLEAR_COLOR[0], z0::WINDOW_CLEAR_COLOR[1], z0::WINDOW_CLEAR_COLOR[2]))} {
        auto hInstance = GetModuleHandle(nullptr);
#ifndef DISABLE_LOG
        _mainThreadId = GetCurrentThreadId();
        if (Application::get().getConfig().loggingMode & LOGGING_WINDOW) {
            createLogWindow(hInstance);
        }
        if (Application::get().getConfig().loggingMode & LOGGING_FILE) {
            _logFile = make_unique<ofstream>("log.txt");
            if(!_logFile->is_open()) {
                die("Error opening log file");
            }
        }
        if (Application::get().getConfig().loggingMode != LOGGING_NONE) {
            log("Log starting");
        }
#endif

        const WNDCLASSEX wincl{
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

        // Getting the dimensions of the primary monitor
        RECT workArea = {};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
        auto screenWidth = workArea.right - workArea.left;
        auto screenHeight = workArea.bottom - workArea.top;

        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;
        int w = CW_USEDEFAULT;
        int h = CW_USEDEFAULT;

        DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER;
        DWORD exStyle = 0;
        switch (applicationConfig.windowMode) {
            case WINDOW_MODE_WINDOWED:{
                RECT rect{
                    .left = 0,
                    .top = 0,
                    .right = static_cast<int>(applicationConfig.windowWidth),  // Desired width of the client area
                    .bottom = static_cast<int>(applicationConfig.windowHeight),
                };
                AdjustWindowRect(&rect, style, FALSE); // Adjust the rect to include the frame
                w = rect.right - rect.left;
                h = rect.bottom - rect.top;
                x = static_cast<int>((screenWidth - w) / 2);
                y = static_cast<int>((screenHeight - h) / 2);
                break;
            }
            case WINDOW_MODE_WINDOWED_MAXIMIZED:{
                style |=  WS_MAXIMIZE;
                break;
            }
            case WINDOW_MODE_FULLSCREEN: {
                DEVMODE dmScreenSettings;
                memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
                dmScreenSettings.dmSize = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth = static_cast<int>(applicationConfig.windowWidth);
                dmScreenSettings.dmPelsHeight =  static_cast<int>(applicationConfig.windowHeight);
                dmScreenSettings.dmBitsPerPel = 32;
                dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
                if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
                    die( "Display mode change failed");
                }
                SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
                screenWidth = workArea.right - workArea.left;
                screenHeight = workArea.bottom - workArea.top;
            }
            case WINDOW_MODE_WINDOWED_FULLSCREEN:{
                exStyle = WS_EX_APPWINDOW;
                style = WS_POPUP | WS_MAXIMIZE;
                x = workArea.left;
                y = workArea.top;
                w = static_cast<int>(screenWidth);
                h = static_cast<int>(screenHeight);
                break;
            }
            default:
                die("Unknown WindowMode");
        }

        rect.top = y;
        rect.left = x;
        rect.bottom = y + h;
        rect.right = x + w;
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
        if (Application::get().getConfig().loggingMode & LOGGING_WINDOW) {
            CloseWindow(_hwndLog);
        }
        if (Application::get().getConfig().loggingMode & LOGGING_FILE) {
            _logFile->close();
        }
#endif
    }

#endif

#ifndef DISABLE_LOG
#ifdef _WIN32

    BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
        static int monitorIndex = 0;
        if (monitorIndex == 0) {
            RECT* pRect = (RECT*)dwData;
            *pRect = *lprcMonitor;
            return FALSE;
        }
        monitorIndex++;
        return TRUE; 
    }
    
    void CopyTextToClipboard(const string& text) {
        if (OpenClipboard(NULL)) {
            EmptyClipboard();

            size_t size = (text.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
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
        auto* window = (z0::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        switch (message) {
            case WM_CREATE: {
                z0::Window::_hwndLogList = CreateWindow(
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
                SendMessage(z0::Window::_hwndLogList, WM_SETFONT, (WPARAM)hFont, TRUE);
                return 0;
            }
            case WM_COMMAND: {
                auto subcommande = HIWORD(wParam);
                if (subcommande == LBN_SELCHANGE) {
                    int index = SendMessage(z0::Window::_hwndLogList, LB_GETCURSEL, 0, 0);
                    if (index != LB_ERR) {
                        auto length = SendMessage(z0::Window::_hwndLogList, LB_GETTEXTLEN, index, 0);
                        TCHAR* buffer = new TCHAR[length+1];
                        SendMessage(z0::Window::_hwndLogList, LB_GETTEXT, index, (LPARAM)buffer);
                        buffer[length] = 0;
                        CopyTextToClipboard(buffer);
                        delete[] buffer;
                    }
                }
                break;
            }
            case WM_SIZE: {
                if (z0::Window::_hwndLogList) {
                    int width = LOWORD(lParam);
                    int height = HIWORD(lParam);
                    MoveWindow(z0::Window::_hwndLogList, 0, 0, width, height, TRUE);
                }
                return 0;
            }
            case WM_DESTROY: {
                DeleteObject(hFont);
            }
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    HWND Window::_hwndLog{nullptr};
    HWND Window::_hwndLogList{nullptr};
    const char szClassNameLog[ ] = "WindowsAppLog";
    list<string> Window::_deferredLogMessages;
    DWORD Window::_mainThreadId;
    unique_ptr<ofstream> Window::_logFile{nullptr};

    void Window::createLogWindow(HMODULE hInstance) {
        RECT secondMonitorRect = { 0 };
        EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&secondMonitorRect);

        const WNDCLASSEX wincl{
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
                secondMonitorRect.left, 
                secondMonitorRect.top,
                400,
                400,
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
        auto logMode = Application::get().getConfig().loggingMode;
        if (logMode == LOGGING_NONE) { return; }
        using namespace chrono;
        auto in_time_t = system_clock::to_time_t(system_clock::now());
        tm tm;
        localtime_s(&tm, &in_time_t);
        wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
        string item = converter.to_bytes(format(L"{:02}:{:02}:{:02}", tm.tm_hour, tm.tm_min, tm.tm_sec));
        item.append(" ");
        item.append(msg);
        if (logMode & LOGGING_WINDOW) {
            // Store the log message in the deferred log queue if we log from another thread
            // because the log window proc run on the main thread
            if ((_hwndLogList != nullptr) && (GetCurrentThreadId() == _mainThreadId)) {
                SendMessage(_hwndLogList, LB_INSERTSTRING, -1, (LPARAM)(item.c_str()));
                int itemCount = SendMessage(_hwndLogList, LB_GETCOUNT, 0, 0);
                SendMessage(_hwndLogList, LB_SETTOPINDEX, itemCount-1, 0);
            } else {
                _deferredLogMessages.push_back(item);
            }
        }
        if (logMode & LOGGING_FILE) {
            *_logFile << item << endl;
        }
    }

    void Window::_processDeferredLog() {
        if (Application::get().getConfig().loggingMode & LOGGING_WINDOW) {
            for(const auto& msg: _deferredLogMessages) {
                SendMessage(_hwndLogList, LB_INSERTSTRING, -1, (LPARAM)(msg.c_str()));
                int itemCount = SendMessage(_hwndLogList, LB_GETCOUNT, 0, 0);
                SendMessage(_hwndLogList, LB_SETTOPINDEX, itemCount-1, 0);
            }
            _deferredLogMessages.clear();
        }
    }
#endif
#endif


}
