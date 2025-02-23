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
#include <Xinput.h>
#endif
#include "z0/libraries.h"

export module z0.Window;

import z0.ApplicationConfig;
import z0.Object;

namespace z0 {

    /**
    * Rendering target Window, one per application
   */
    export class Window: public Object {
    public:
        /**
         * Returns the width of the client area of the Window, in pixels
         */
        [[nodiscard]] uint32_t getWidth() const { return width; }

        /**
         * Returns the height of the client area of the Window, in pixels
         */
        [[nodiscard]] uint32_t getHeight() const { return height; }

        /**
         * Closes the Window, effectively quitting the application, at the end of the current frame
         */
        void close() { closing = true; }

        /**
        * Returns true if we need to stop the Application main loop and quit the application
        */
        [[nodiscard]] bool shouldClose() const { return closing; }

        [[nodiscard]] string toString() const override;

    private:
        // width of the client area
        uint32_t width;
        // height of the client area
        uint32_t height;
        // If true close the Window and quit the application at the end of the current frame
        bool closing{false};

#ifdef _WIN32
        HWND hwnd;
        RECT rect;
        HBRUSH background;
#endif

    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

#ifdef _WIN32
        static bool resettingMousePosition;
        void _setSize(int width, int height);
        [[nodiscard]] HWND _getHandle() const { return hwnd; };
        [[nodiscard]] RECT _getRect() const { return rect; }
#endif
        explicit Window(const ApplicationConfig& applicationConfig);
        ~Window() override;
    };

}
