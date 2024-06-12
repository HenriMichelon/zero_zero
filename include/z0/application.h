#pragma once

namespace z0 {

    class GWindow;
    class GManager;
    class SceneRenderer;
    class VectorRenderer;
    class Camera;

    /**
     * Global application.
     * Automaticaly instanciated by the Z0_APP macro
     */
    class Application final: public Object {
    public:
        /**
         * Get the application singleton
         * @return the global application object
         */
        static Application& get();

        /**
         * Add a GUI window to the window manager for display
         * @param window    The window to display, must not be already added to the window manager
         */
        void addWindow(const shared_ptr<GWindow>&window);

        /**
         * Remove the window from the window manager
         * @param window    The window to remove, must be added to the window manager before
         */
        void removeWindow(const shared_ptr<GWindow>&window);

        /**
         * Exit the application by closing the window (will wait for the current frame to be terminated)
         */
        void quit();

        /**
         * Get the startup configuration
         * @return the global configuration given when the application was instancied
         */
        const ApplicationConfig& getConfig() const;

        /**
         * Get the current display window
         * @return The main window
         */
        const Window& getWindow() const;

        /**
         * Get the frames per seconds
         * @return The average FPS
         */
        uint32_t getFPS() const { return fps; }

        /**
         * Check if the scene is paused, in respect for \ProcessMode
         * @return  \true if the current scene is paused
         */
        bool isPaused() const { return paused; }

        /**
         * Pause or resume the current scene
         * @param pause the new state
         *  - \true pause the scene
         *  - \false resume the scene
         */
        void setPaused(bool pause);

        /** 
         * Change the current scene
         * @param node The new scene, must not have a parent
         */
        void setRootNode(const shared_ptr<Node>& node);

        /**
         * Change the current camera
         * @param camera the camera to activate, must be in a scene
         */
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

        Device& _getDevice() { return *device; }
        VkInstance _getVkInstance() const { return vkInstance; }

        void _addNode(const shared_ptr<Node>& node);
        void _removeNode(const shared_ptr<Node>& node);
        void _onInput(InputEvent& inputEvent);

        JPH::BodyInterface& _getBodyInterface() { return physicsSystem.GetBodyInterface(); }
        JPH::PhysicsSystem& _getPhysicsSystem() { return physicsSystem; }
        BPLayerInterfaceImpl& _getBPLayerInterfaceImpl() { return broad_phase_layer_interface; }

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        explicit Application(const ApplicationConfig& applicationConfig, const shared_ptr<Node>& rootNode);
        virtual ~Application();
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