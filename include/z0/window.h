#pragma once

#include "z0/object.h"
#include "z0/application_config.h"

namespace z0 {

    class Application;

    class Window: public Object {
    public:
        explicit Window(const ApplicationConfig& applicationConfig);
        virtual ~Window();

        static uint32_t getScreenWidth() { return screenWidth; }
        static uint32_t getScreenHeight() { return screenHeight; }

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

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
        void _setSize(int width, int height);
        HWND _getHandle() const { return hwnd; };
#endif
    };

}