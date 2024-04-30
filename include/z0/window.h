#pragma once

#include "z0/object.h"

namespace z0 {

    class Window: public Object {
    public:
        virtual ~Window();

        static uint32_t getScreenWidth() { return screenWidth; };
        static uint32_t getScreenHeight() { return screenHeight; };

    protected:
        string toString() const override;

    private:
        static uint32_t screenWidth;
        static uint32_t screenHeight;
        uint32_t width;
        uint32_t height;

#ifdef _WIN32
        HWND hwnd;
        HBRUSH background;
#endif

    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

#ifdef _WIN32
        // accessed by global function WinMain
        Window(HINSTANCE hThisInstance);
        void _setSize(int width, int height);
#endif
    };

}