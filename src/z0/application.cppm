/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <cassert>

#include "z0/libraries.h"

export module z0.Application;

import z0.Constants;
import z0.ApplicationConfig;
import z0.InputEvent;
import z0.Physics;
import z0.Signal;
import z0.Tools;
import z0.Window;

import z0.nodes.Camera;
import z0.nodes.Node;

import z0.resources.Material;

import z0.ui.Manager;
import z0.ui.Window;

namespace z0 {

    /**
     * Global application.<br>
     * Automatically instantiated by the `Z0_APP(CONFIG, ROOTNODE)` macro.<br>
     */
    export class Application {
    public:
        static constexpr uint32_t MAX_ASYNC_CALLS = 3;

        /**
         * Returns the application singleton
         */
        static Application &get() {
            assert(_instance != nullptr);
            return *_instance;
        }

        /**
         * Adds a GUI Window to the Window manager for display
         * @param window    The window to display, must not be already added to the window manager
         */
        void add(const shared_ptr<ui::Window> &window) const;

        /**
         * Removes the Window from the Window manager
         * @param window    The window to remove, must be added to the window manager before
         */
        void remove(const shared_ptr<ui::Window> &window) const;

        /**
         * Exits the application by closing the Window (will wait for the current frame to be terminated)
         */
        void quit() const;

        /**
         * Returns the startup configuration
         * @return the global configuration given when the application was instanced
         */
        [[nodiscard]] const ApplicationConfig &getConfig() const {
            return applicationConfig;
        }

        /**
         * Returns the current display Window
         * @return The main window
         */
        [[nodiscard]] const Window &getWindow() const {
            return *window;
        }

        /**
         * Returns the frames per seconds
         * @return The average FPS
         */
        [[nodiscard]] uint32_t getFPS() const { return fps; }

        /**
         * Checks if the scene is paused, in respect for z0::ProcessMode
         * @return  true if the current scene is paused
         */
        [[nodiscard]] bool isPaused() const { return paused; }

        /**
         * Pause or resume the current scene
         * @param pause the new state
         *  - true pause the scene
         *  - false resume the scene
         */
        void setPaused(bool pause);

        /**
         * Changes the current scene
         * @param node The new scene, must not have a parent
         */
        void setRootNode(const shared_ptr<Node> &node);

        /**
         * Changes the current camera
         * @param camera the camera to activate, must be in a scene
         */
        void activateCamera(const shared_ptr<Camera> &camera);

        /**
         * Returns the physics system gravity
         */
        [[nodiscard]] vec3 getGravity() const;

        /**
         * Returns the number of bytes of dedicated video memory that are not shared with the CPU.
         */
        [[nodiscard]] virtual uint64_t getDedicatedVideoMemory() const = 0;

        /**
         * Returns the video adapter description
         */
        [[nodiscard]] virtual const string &getAdapterDescription() const = 0;

        /**
         * Returns the application’s current video memory usage, in bytes.
         */
        [[nodiscard]] virtual uint64_t getVideoMemoryUsage() const = 0;

        /**
         * Enable or disable shadow casting for the entire scene, applied when adding nodes
         */
        virtual void setShadowCasting(bool) const = 0;

        /**
         * Returns the global Window manager
         */
        ui::Manager &getWindowManager() const { return *windowManager; }

        /**
         * Return the vector renderer size ratio
         */
        const vec2 &getVectorRatio() const { return vectorRatio; }

        /**
         * Returns the rendering Window aspect ratio
         */
        [[nodiscard]] virtual float getAspectRatio() const = 0;

        /**
        * Add a lambda expression in the deferred calls queue.<br>
        * They will be called before the next frame, after the scene pre-drawing updates
        * where nodes are added/removed from the drawing lists (for all the frames in flight).
        */
        template<typename Lambda>
        void callDeferred(Lambda lambda) {
            auto lock = lock_guard(deferredCallsMutex);
            deferredCalls.push_back(lambda);
        }

        /**
         * Starts a new thread that can access the GPU/VRAM.<br>
         * Use this instead of starting a thread manually because the rendering system needs
         * to wait for all the threads completion before releasing resources.
         */
        template <typename Lambda>
        void callAsync(Lambda lambda) {
            threadedCalls.push_back(jthread(lambda));
        }

        /**
         * Returns the meshes outlining materials
         */
        inline OutlineMaterials& getOutlineMaterials() const { return *outlineMaterials; }

        /**
         * If the debug renderer is enabled is the application configuration,
         * show or hide the debug.
         */
        void setDisplayDebug(bool display);

        /**
         *
         * Return `true` if the debug renderer display anything.
         */
        inline bool getDisplayDebug() const { return displayDebug; }

    private:
        // State of the current scene
        bool paused{false};
        // State of the main loop
        bool stopped{true};
        // Average FPS,
        uint32_t fps{0};
        // The current frame in flight
        uint32_t currentFrame{0};
        // vector renderer size ratios
        vec2 vectorRatio;

        /*
         * Main loop members
         */
        using Clock = chrono::steady_clock;
        static constexpr float dt = 1.0f/60.0f;
        double t = 0.0;
        double currentTime = 0.0;
        double accumulator = 0.0;
        uint32_t frameCount = 0;

        /*
         * Physic system members
         */
        JPH::PhysicsSystem                   physicsSystem;
        ContactListener                      contactListener;
        BPLayerInterfaceImpl                 broad_phase_layer_interface;
        ObjectVsBroadPhaseLayerFilterImpl    object_vs_broadphase_layer_filter;
        ObjectLayerPairFilterImpl            object_vs_object_layer_filter;
        unique_ptr<JPH::TempAllocatorImpl>   temp_allocator;
        unique_ptr<JPH::JobSystemThreadPool> job_system;

        // Called on startup and after each root node change
        void start();

        // Prepare and draw a frame
        void drawFrame();

        // Reset the allocated nodes of the tree node
        void cleanup(shared_ptr<Node> &node);

        // Recursively reset the allocated nodes of the tree node
        void ready(const shared_ptr<Node> &node);

        // Recursively call _onReady() on a tree node
        void pause(const shared_ptr<Node> &node);

        // Recursively call onProcess() on a tree node
        void process(const shared_ptr<Node> &node, float alpha);

        // Recursively call onPhysicsProcess() on a tree node
        void physicsProcess(const shared_ptr<Node> &node, float delta);

        // Recursively call onInput() on a tree node
        bool input(const shared_ptr<Node> &node, InputEvent &inputEvent);

        // Registers all nodes types
        void registerTypes() const;


    protected:
        // The global startup configuration parameters
        const ApplicationConfig &applicationConfig;
        // The global display Window
        unique_ptr<Window> window;
        // The global UI Window manager
        unique_ptr<ui::Manager> windowManager;
        // The current scene
        shared_ptr<Node> rootNode;
        mutex rootNodeMutex;
        // Mesh outlining materials
        unique_ptr<OutlineMaterials> outlineMaterials;
        // Number of seconds since last FPS update
        float elapsedSeconds{0.0f};
        // If physics debug is enabled in the application config, it can be disabled at runtime
        bool displayDebug{true};

        struct FrameData {
            // Deferred list of nodes added to the current scene, processed before each frame
            list<shared_ptr<Node>> addedNodes;
            list<shared_ptr<Node>> addedNodesAsync;
            // Deferred list of nodes removed from the current scene, processed before each frame
            list<shared_ptr<Node>> removedNodes;
            list<shared_ptr<Node>> removedNodesAsync;
            // Camera to activate next frame
            shared_ptr<Camera> activeCamera;
        };
        vector<FrameData> frameData;
        mutex frameDataMutex;
        bool doDeferredUpdates{true};

        // Deferred nodes calls, to be called after processDeferredUpdates()
        list<function<void()>> deferredCalls;
        mutex deferredCallsMutex;

        list<jthread> threadedCalls;
        mutex threadedCallsMutex;

        explicit Application(const ApplicationConfig &applicationConfig, const shared_ptr<Node> &rootNode);

        void init();

        virtual void initRenderingSystem() = 0;

        virtual void renderFrame(uint32_t currentFrame) = 0;

        virtual void waitForRenderingSystem() = 0;

        virtual void stopRenderingSystem() = 0;

        // Process scene updates before drawing a frame
        virtual void processDeferredUpdates(uint32_t currentFrame) = 0;

    public:
        // The following members are accessed by global function WinMain
        // Application singleton
        static Application *_instance;
#ifdef _WIN32
        // The Main Loop, process Windows message and draw a frame
        void _mainLoop();
#endif
#ifndef DISABLE_LOG
        FILE* _logFile;
#endif
        // Pause/resume the main loop
        void _stop(const bool stop) { stopped = stop; };

        JPH::BodyInterface &_getBodyInterface() { return physicsSystem.GetBodyInterface(); }

        JPH::PhysicsSystem &_getPhysicsSystem() { return physicsSystem; }

        ObjectLayerPairFilterImpl &_getObjectLayerPairFilter() { return object_vs_object_layer_filter; }

        BPLayerInterfaceImpl &_getBPLayerInterfaceImpl() { return broad_phase_layer_interface; }

        unique_ptr<JPH::TempAllocatorImpl> &_getTempAllocator() { return temp_allocator; }

        // Add a node to the current scene
        void _addNode(const shared_ptr<Node> &node, bool async);

        // Remove a node from the current scene
        void _removeNode(const shared_ptr<Node> &node, bool async);

        void _lockDeferredUpdate();

        void _unlockDeferredUpdate();

        // Propagate input event to the UI Window manager and to the current scene tree
        void _onInput(InputEvent &inputEvent);

        Application(const Application &) = delete;

        Application &operator=(const Application &) = delete;

        virtual ~Application();
    };

    export inline Application& app() { return Application::get(); }

}
