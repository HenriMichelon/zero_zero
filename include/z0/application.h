#pragma once

namespace z0 {

    class GWindow;
    class GManager;
    class SceneRenderer;
    class VectorRenderer;
    class BasePostprocessingRenderer;
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
         * Check if the scene is paused, in respect for z0::ProcessMode
         * @return  true if the current scene is paused
         */
        bool isPaused() const { return paused; }

        /**
         * Pause or resume the current scene
         * @param pause the new state
         *  - true pause the scene
         *  - false resume the scene
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

        /**
         * Get the physics system gravity
         */
        vec3 getGravity() const;

    private:
        // The global startup configuration parameters
        const ApplicationConfig& applicationConfig;
        // The current scene
        shared_ptr<Node> rootNode;
        // The global display window
        unique_ptr<Window> window;
        // The global UI Window manager
        unique_ptr<GManager> windowManager;
        // The Vulkan device helper object
        unique_ptr<Device> device;
        // The Vulkan global instance
        VkInstance vkInstance;
        // Used to redirect validation layers to the logging system
        VkDebugUtilsMessengerEXT debugMessenger;
        // State of the current scene
        bool paused{false};
        // State of the main loop
        bool stopped{true};
        // Average FPS,
        uint32_t fps{0};
        // The Main renderer
        shared_ptr<SceneRenderer> sceneRenderer;
        // The 2D vector renderer used for the UI
        shared_ptr<VectorRenderer> vectorRenderer;
        // Defered list of nodes added to the current scene, processed before each frame
        vector<shared_ptr<Node>> addedNodes{};
        // Defered list of nodes removed from the current scene, processed before each frame
        vector<shared_ptr<Node>> removedNodes{};

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

        // Called on startup and after each root node change
        void start();
        // Prepare and draw a frame
        void drawFrame();
        // Reset the allocated nodes of the tree node
        void cleanup(shared_ptr<Node>& node);
        // Recusively reset the allocated nodes of the tree node
        void ready(const shared_ptr<Node>& node);
        // Recusively call _onReady() on a tree node
        void pause(const shared_ptr<Node>& node);
        // Recusively call onProcess() on a tree node
        void process(const shared_ptr<Node>& node, float alpha);
        // Recusively call onPhysicsProcess() on a tree node
        void physicsProcess(const shared_ptr<Node>& node, float delta);
        // Recusively call onInput() on a tree node
        bool input(const shared_ptr<Node>& node, InputEvent& inputEvent);

    public:
        // The following members are accessed by global function WinMain
        // Application singleton
        static Application* _instance;
#ifdef _WIN32
        // The Main Loop, process Windows message and draw a frame
        void _mainLoop();
#endif
        // Pause/resume the main loop
        void _stop(bool stop) { stopped = stop; };

        // Internal accessor/modifiers
        Device& _getDevice() { return *device; }
        VkInstance _getVkInstance() const { return vkInstance; }
        JPH::BodyInterface& _getBodyInterface() { return physicsSystem.GetBodyInterface(); }
        JPH::PhysicsSystem& _getPhysicsSystem() { return physicsSystem; }
        BPLayerInterfaceImpl& _getBPLayerInterfaceImpl() { return broad_phase_layer_interface; }
        unique_ptr<JPH::TempAllocatorImpl>& _getTempAllocator() { return temp_allocator; }

        // Add a node to the current scene
        void _addNode(const shared_ptr<Node>& node);
        // Remove a node from the current scene
        void _removeNode(const shared_ptr<Node>& node);
        // Propagate input event to the UI Window manager and to the current scene tree
        void _onInput(InputEvent& inputEvent);

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;
        explicit Application(const ApplicationConfig& applicationConfig, const shared_ptr<Node>& rootNode);
        virtual ~Application();
   };

#ifdef _WIN32
    /**
     * Application startup macro
     * @param CONFIG An ApplicationConfig object
     * @param ROOTNODE The root Node of the startup scene
     */
    #define Z0_APP(CONFIG, ROOTNODE) \
        z0::Application _z0_app(CONFIG, ROOTNODE); \
        int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) { \
            if (z0::Application::_instance == nullptr) z0::die("No Application object found"); \
            z0::Application::get()._mainLoop(); \
            return 0; \
        }
#endif

}