#include "z0/application.h"

#ifdef _WIN32
char szClassName[ ] = "WindowsApp";
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* window = (z0::Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (message)
    {
        case WM_CREATE:
            // Save the Window pointer passed to CreateWindowEx
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
            return 0;
        case WM_SIZING:
        case WM_ACTIVATE: {
            // Get window content size
            RECT rect = {};
            GetClientRect(hwnd, &rect);
            window->_setSize( rect.right - rect.left, rect.bottom - rect.top);
            break;
        }
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
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

    Window::Window(HINSTANCE hThisInstance):
        width{0},
        height{0},
        background{CreateSolidBrush(RGB(z0::WINDOW_CLEAR_COLOR[0], z0::WINDOW_CLEAR_COLOR[1], z0::WINDOW_CLEAR_COLOR[2]))} {

        z0::Application& application = z0::Application::get();

        const WNDCLASSEX wincl{
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_DBLCLKS,
            .lpfnWndProc = WindowProcedure,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hThisInstance,
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

        DWORD style = 0;
        DWORD exStyle = 0;
        switch (application.getConfig().windowMode) {
            case WINDOW_MODE_WINDOWED:{
                style = WS_OVERLAPPEDWINDOW;
                RECT rect{
                    .left = 0,
                    .top = 0,
                    .right = static_cast<int>(application.getConfig().windowWidth),  // Desired width of the client area
                    .bottom = static_cast<int>(application.getConfig().windowHeight),
                };
                AdjustWindowRect(&rect, style, FALSE); // Adjust the rect to include the frame
                w = rect.right - rect.left;
                h = rect.bottom - rect.top;
                x = static_cast<int>((screenWidth - w) / 2);
                y = static_cast<int>((screenHeight - h) / 2);
                break;
            }
            case WINDOW_MODE_WINDOWED_MAXIMIZED:{
                style = WS_OVERLAPPEDWINDOW | WS_MAXIMIZE;
                break;
            }
            case WINDOW_MODE_FULLSCREEN: {
                DEVMODE dmScreenSettings;
                memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
                dmScreenSettings.dmSize = sizeof(dmScreenSettings);
                dmScreenSettings.dmPelsWidth = static_cast<int>(application.getConfig().windowWidth);
                dmScreenSettings.dmPelsHeight =  static_cast<int>(application.getConfig().windowHeight);
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

        hwnd = CreateWindowEx(
                exStyle,
                szClassName,
                application.getConfig().appName.c_str(),
                style,
                x,
                y,
                w,
                h,
                HWND_DESKTOP,
                nullptr,
                hThisInstance,
                this
        );
        if (hwnd == nullptr) die("Cannot create Window", std::to_string(GetLastError()));

        ShowWindow(hwnd, SW_SHOW );
        UpdateWindow(hwnd);
    }

    Window::~Window() {
        DeleteObject(background);
    }

#endif


}
