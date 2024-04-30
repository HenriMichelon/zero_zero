#pragma once

#include "z0/window.h"

#include <filesystem>
#include <cassert>

namespace z0 {

    /*enum MSAA {
        MSAA_AUTO       = 0,
        MSAA_2X         = 1,
        MSAA_4X         = 2,
        MSAA_8X         = 3,
    };*/

    struct ApplicationConfig {
        string appName             = "MyApp";
        filesystem::path appDir    = ".";
        WindowMode windowMode      = WINDOW_MODE_WINDOWED;
        uint32_t windowWidth       = 800;
        uint32_t windowHeight      = 600;
        /*MSAA msaa                  = MSAA_AUTO;
        float gamma                = 1.0f;
        float exposure             = 1.0f;*/
    };

    class Application: public Object {
    public:
        explicit Application(const ApplicationConfig& applicationConfig);

        static Application& get();

        const ApplicationConfig& getConfig() { return applicationConfig; }
        [[nodiscard]] Window& getWindow() const;

    private:
        const ApplicationConfig& applicationConfig;


    public:
        // The following members are accessed by global function WinMain
        static Application* _instance;
#ifdef _WIN32
        MSG _messages;
        unique_ptr<Window> _window;
        void _mainLoop();
#endif

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
    };

}