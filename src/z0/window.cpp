#include "z0/application.h"

#ifdef _WIN32
char szClassName[ ] = "WindowsApp";
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ACTIVATE: {
            // Clear window background
            PAINTSTRUCT ps2;
            HDC hdc = BeginPaint(hwnd, &ps2);
            HBRUSH brush = CreateSolidBrush(
                    RGB(z0::WINDOW_CLEAR_COLOR[0], z0::WINDOW_CLEAR_COLOR[1], z0::WINDOW_CLEAR_COLOR[2]));
            RECT rect = {};
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, brush);
            DeleteObject(brush);
            EndPaint(hwnd, &ps2);
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

#ifdef _WIN32
    Window::Window(HINSTANCE hThisInstance) {
        z0::Application& application = z0::Application::get();

        WNDCLASSEX wincl;

        wincl.hInstance = hThisInstance;
        wincl.lpszClassName = szClassName;
        wincl.lpfnWndProc = WindowProcedure;
        wincl.style = CS_DBLCLKS;
        wincl.cbSize = sizeof (WNDCLASSEX);

        wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
        wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
        wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
        wincl.lpszMenuName = NULL;
        wincl.cbClsExtra = 0;
        wincl.cbWndExtra = 0;
        wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

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
                exStyle = WS_EX_OVERLAPPEDWINDOW;
                style = WS_OVERLAPPEDWINDOW;
                x = static_cast<int>((screenWidth- application.getConfig().windowWidth) / 2);
                y = static_cast<int>((screenHeight- application.getConfig().windowHeight) / 2);
                w = static_cast<int>(application.getConfig().windowWidth);
                h = static_cast<int>(application.getConfig().windowHeight);
                break;
            }
            case WINDOW_MODE_WINDOWED_MAXIMIZED:{
                exStyle = WS_EX_OVERLAPPEDWINDOW;
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
                nullptr
        );
        if (hwnd == nullptr) die("Cannot create Window", std::to_string(GetLastError()));

        ShowWindow(hwnd, SW_SHOW );
        UpdateWindow(hwnd);
    }

#endif


}
