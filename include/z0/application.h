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
        void removeNode(const shared_ptr<Node>& node);
        void activateCamera(const shared_ptr<Camera>& camera);

    private:
        const ApplicationConfig& applicationConfig;
        shared_ptr<Node> rootNode;
        unique_ptr<Window> window;
        unique_ptr<Device> device;
        VkInstance vkInstance;
        bool paused{false};
        shared_ptr<SceneRenderer> sceneRenderer;
        vector<shared_ptr<Node>> addedNodes{};
        vector<shared_ptr<Node>> removedNodes{};

        using Clock = std::chrono::steady_clock;
        static constexpr float dt = 0.01;
        double t = 0.0;
        double currentTime = chrono::duration_cast<chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double accumulator = 0.0;
        uint32_t frameCount = 0;
        float elapsedSeconds = 0.0;

        void start();
        void drawFrame();
        void end();

        void cleanup(shared_ptr<Node>& node);
        void ready(const shared_ptr<Node>& node);
        void process(const shared_ptr<Node>& node, float alpha);
        void physicsProcess(const shared_ptr<Node>& node, float delta);

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