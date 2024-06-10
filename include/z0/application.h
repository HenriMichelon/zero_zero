#pragma once

namespace z0 {

    class GWindow;
    class GManager;
    class SceneRenderer;
    class VectorRenderer;
    class Camera;

    class Application: public Object {
    public:
        explicit Application(const ApplicationConfig& applicationConfig, const shared_ptr<Node>& rootNode);
        virtual ~Application();

        static Application& get();
        void addWindow(const shared_ptr<GWindow>&);
        void removeWindow(const shared_ptr<GWindow>&);
        void quit();

        const ApplicationConfig& getConfig() const { return applicationConfig; }
        const Window& getWindow() const;
        Device& getDevice() { return *device; }
        VkInstance getVkInstance() const { return vkInstance; }
        uint32_t getFPS() const { return fps; }

        bool isPaused() const { return paused; }
        void setPaused(bool pause);
        void onInput(InputEvent& inputEvent);
        void setRootNode(const shared_ptr<Node>& node);
        void activateCamera(const shared_ptr<Camera>& camera);

    private:
        const ApplicationConfig& applicationConfig;
        shared_ptr<Node> rootNode;
        unique_ptr<Window> window;
        unique_ptr<Device> device;
        VkInstance vkInstance;
        VkDebugUtilsMessengerEXT debugMessenger;
        bool paused{false};
        bool stopped{true};
        uint32_t fps{0};
        shared_ptr<SceneRenderer> sceneRenderer;
        shared_ptr<VectorRenderer> vectorRenderer;
        vector<shared_ptr<Node>> addedNodes{};
        vector<shared_ptr<Node>> removedNodes{};
        unique_ptr<GManager> windowManager;

        /*
         * Main loop members
         */
        using Clock = std::chrono::steady_clock;
        static constexpr float dt = 0.01;
        double t = 0.0;
        double currentTime = chrono::duration_cast<chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double accumulator = 0.0;
        uint32_t frameCount = 0;
        float elapsedSeconds = 0.0;

        /*
         * Physic system members
         */
        JPH::PhysicsSystem physicsSystem;
        ContactListener contactListener;
        BPLayerInterfaceImpl broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
        ObjectLayerPairFilterImpl object_vs_object_layer_filter;
        unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        unique_ptr<JPH::JobSystemThreadPool> job_system;

        void start();
        void drawFrame();
        void end();

        void cleanup(shared_ptr<Node>& node);
        void ready(const shared_ptr<Node>& node);
        void pause(const shared_ptr<Node>& node);
        void process(const shared_ptr<Node>& node, float alpha);
        void physicsProcess(const shared_ptr<Node>& node, float delta);
        bool input(const shared_ptr<Node>& node, InputEvent& inputEvent);

    public:
        // The following members are accessed by global function WinMain
        static Application* _instance;
#ifdef _WIN32
        void _mainLoop();
#endif
        void _stop(bool stop) { stopped = stop; };

        void _addNode(const shared_ptr<Node>& node);
        void _removeNode(const shared_ptr<Node>& node);

        JPH::BodyInterface& _getBodyInterface() { return physicsSystem.GetBodyInterface(); }
        JPH::PhysicsSystem& _getPhysicsSystem() { return physicsSystem; }
        BPLayerInterfaceImpl& _getBPLayerInterfaceImpl() { return broad_phase_layer_interface; }

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
    };

#ifdef _WIN32
    #define Z0_APP(CONFIG, ROOTNODE) \
        z0::Application _z0_app(CONFIG, ROOTNODE); \
        int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) { \
            if (z0::Application::_instance == nullptr) z0::die("No Application object found"); \
            z0::Application::get()._mainLoop(); \
            return 0; \
        }
#endif

}