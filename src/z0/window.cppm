module;
#include "z0/modules.h"
#ifdef _WIN32
    #include <Xinput.h>
    #include <dinput.h>
#endif

export module Z0:Window;

import :ApplicationConfig;
import :Object;

namespace z0 {

    /**
       * Rendering target window, one per application
       */
    export class Window: public Object {
    public:
        /**
         * Returns the width of the client area of the window, in pixels
         */
        [[nodiscard]] uint32_t getWidth() const { return width; }

        /**
         * Returns the height of the client area of the window, in pixels
         */
        [[nodiscard]] uint32_t getHeight() const { return height; }

        /**
         * Closes the window, effectively quitting the application, at the end of the current frame
         */
        void close() { closing = true; }

        /*
        * Returns true if we need to stop the Application main loop and quit the application
        */
        [[nodiscard]] bool shouldClose() const { return closing; }

        string toString() const override;

    private:
        // width of the client area
        uint32_t width;
        // height of the client area
        uint32_t height;
        // If true close the window and quit the application at the end of the current frame
        bool closing{false};

#ifdef _WIN32
        HWND hwnd;
        RECT rect;
        HBRUSH background;
#ifndef DISABLE_LOG
        static HWND _hwndLog;
        static DWORD _mainThreadId;
        static list<string> _deferredLogMessages;
        void createLogWindow(HMODULE);
#endif
#endif

    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

#ifdef _WIN32
#ifndef DISABLE_LOG
        static HWND _hwndLogList;
        static unique_ptr<ofstream> _logFile;
        static void _log(string);
        static void _processDeferredLog();
#endif

        void _setSize(int width, int height);
        [[nodiscard]] HWND _getHandle() const { return hwnd; };
        [[nodiscard]] RECT _getRect() const { return rect; }
#endif
        explicit Window(const ApplicationConfig& applicationConfig);
        virtual ~Window();
    };

}
