#pragma once

namespace z0 {

    /**
     * Rendering target window, one per application
     */
    class Window: public Object {
    public:
        /**
         * Get the width of the client area of the window, in pixels
         */
        uint32_t getWidth() const { return width; }

        /**
         * Get the height of the client area of the window, in pixels
         */
        uint32_t getHeight() const { return height; }

        /**
         * Close the window, effectively quitting the application, at the end of the current frame
         */
        void close() { closing = true; }

        /*
        * Returns true if we need to stop the Application main loop and quit the application
        */
        bool shouldClose() const { return closing; }

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
        static HWND _hwndLog;
        static DWORD _mainThreadId;
        static list<string> _deferredLogMessages;
        void createLogWindow(HMODULE);
#endif

    public:
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

#ifdef _WIN32
        static HWND _hwndLogList;
        static ofstream _logFile;
        static void _log(string);
        static void _processDeferredLog();

        void _setSize(int width, int height);
        HWND _getHandle() const { return hwnd; };
        RECT _getRect() const { return rect; }
#endif
        explicit Window(const ApplicationConfig& applicationConfig);
        virtual ~Window();
   };

}