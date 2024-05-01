#pragma once

#include "z0/device.h"
#include "z0/nodes/node.h"

#include <filesystem>
#include <cassert>

namespace z0 {

    class Application: public Object {
    public:
        explicit Application(const ApplicationConfig& applicationConfig, const shared_ptr<Node>& rootNode);
        virtual ~Application();

        static Application& get();

        const ApplicationConfig& getConfig() const { return applicationConfig; }
        const Window& getWindow() const;
        Device& getDevice() { return *device; }
        VkInstance getVkInstance() const { return vkInstance; }
        bool isPaused() const { return paused; }
        void setPaused(bool pause) { paused = pause; }

    private:
        const ApplicationConfig& applicationConfig;
        shared_ptr<Node> rootNode;
        unique_ptr<Window> window;
        unique_ptr<Device> device;
        VkInstance vkInstance;
        bool paused{false};

    public:
        // The following members are accessed by global function WinMain
        static Application* _instance;
#ifdef _WIN32
        MSG _messages;
        void _mainLoop();
#endif

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
    };

}