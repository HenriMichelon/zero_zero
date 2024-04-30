#pragma once

#include "z0/object.h"

namespace z0 {

    enum WindowMode {
        WINDOW_MODE_WINDOWED            = 0,
        WINDOW_MODE_WINDOWED_MAXIMIZED  = 1,
        WINDOW_MODE_WINDOWED_FULLSCREEN = 2,
        WINDOW_MODE_FULLSCREEN          = 3,
    };

    const int WINDOW_CLEAR_COLOR[] { 0, 0, 0 };

    class Window {
    public:
        static uint32_t getScreenWidth() { return screenWidth; };
        static uint32_t getScreenHeight() { return screenHeight; };

    private:
        static uint32_t screenWidth;
        static uint32_t screenHeight;

#ifdef _WIN32
        HWND hwnd;
#endif

    public:
#ifdef _WIN32
        // accessed by global function WinMain
        Window(HINSTANCE hThisInstance);
#endif
    };

}