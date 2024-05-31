#include "z0/base.h"
#include "z0/input.h"
#include "z0/input_event.h"

#ifdef _WIN32
char szClassName[ ] = "WindowsApp";

int _getKeyboardModifiers() {
    int modifiers = 0;
    if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= z0::KEY_MODIFIER_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= z0::KEY_MODIFIER_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000) modifiers |= z0::KEY_MODIFIER_ALT;
    return modifiers;
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static float lastMouseX = -1.0f;
    static float lastMouseY = -1.0f;
    auto* window = (z0::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto& app = z0::Application::get();
    switch (message)
    {
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
                // Mark the window for closing
                window->close();
                // cancel the default close operation
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
            app.onInput(event);
            break;
        }
        case WM_KEYUP: {
            auto scanCode = static_cast<z0::OsKey>((lParam & 0x00FF0000) >> 16);
            auto key = z0::Input::osKeyToKey(scanCode);
            z0::Input::_keyPressedStates[key] = false;
            z0::Input::_keyJustPressedStates[key] = false;
            z0::Input::_keyJustReleasedStates[key] = true;
            auto event = z0::InputEventKey{key, false, static_cast<int>(lParam & 0xFFFF), _getKeyboardModifiers()};
            app.onInput(event);
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
            app.onInput(event);
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
            app.onInput(event);
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
            app.onInput(event);
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
            app.onInput(event);
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
            app.onInput(event);
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
            app.onInput(event);
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
                app.onInput(event);
            } else {
                auto event = z0::InputEventMouseMotion(xPos, yPos, 0, 0);
                app.onInput(event);
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

    uint32_t Window::screenWidth = 0;
    uint32_t Window::screenHeight = 0;

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
        screenWidth = workArea.right - workArea.left;
        screenHeight = workArea.bottom - workArea.top;

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
        if (hwnd == nullptr) die("Cannot create Window", to_string(GetLastError()));

        RAWINPUTDEVICE rid[1];
        rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
        rid[0].dwFlags = 0;
        rid[0].hwndTarget = 0;
        if (!RegisterRawInputDevices(rid, 1, sizeof(RAWINPUTDEVICE))) {
            die("Failed to register raw input devices.", to_string(GetLastError()));
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    Window::~Window() {
        DeleteObject(background);
    }

#endif


}
