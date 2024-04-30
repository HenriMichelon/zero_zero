#pragma once

#include "z0/window.h"
#include "z0/nodes/node.h"

#include <filesystem>
#include <cassert>

namespace z0 {

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
        explicit Application(const ApplicationConfig& applicationConfig, const shared_ptr<Node>& rootNode);

        static Application& get();

        const ApplicationConfig& getConfig() { return applicationConfig; }
        [[nodiscard]] Window& getWindow() const;

    private:
        const ApplicationConfig& applicationConfig;
        shared_ptr<Node> rootNode;

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