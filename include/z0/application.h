#pragma once

#include "z0/device.h"
#include "z0/nodes/node.h"
#include "z0/renderers/scene_renderer.h"

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

        void addNode(const shared_ptr<Node>& node);

    private:
        const ApplicationConfig& applicationConfig;
        shared_ptr<Node> rootNode;
        unique_ptr<Window> window;
        unique_ptr<Device> device;
        VkInstance vkInstance;
        bool paused{false};
        shared_ptr<SceneRenderer> sceneRenderer;
        vector<shared_ptr<Node>> addedNodes{};

        void start();
        void drawFrame();
        void end();

    public:
        // The following members are accessed by global function WinMain
        static Application* _instance;
#ifdef _WIN32
        void _mainLoop();
#endif

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
    };

}