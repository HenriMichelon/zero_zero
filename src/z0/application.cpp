module;
#include <cassert>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/RegisterTypes.h>

module z0;

import :Node;
import :Application;
import :Input;
import :InputEvent;
import :TypeRegistry;
import :Window;
import :Material;
import :GManager;
import :Camera;
import :Character;
import :CollisionArea;
import :DirectionalLight;
import :Environment;
import :OmniLight;
import :RayCast;
import :KinematicBody;
import :RigidBody;
import :Skybox;
import :StaticBody;
import :Viewport;

namespace z0 {

    // Unique application instance
    Application *Application::_instance = nullptr;

    Application::Application(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        applicationConfig{appConfig}, rootNode{node} {
        assert(_instance == nullptr && rootNode != nullptr);
        _instance = this;
        frameData.resize(applicationConfig.framesInFlight);
        // The rendering window
        window = make_unique<Window>(applicationConfig);
    }

    void Application::init() {
        // Compute the scale ratios for the vector renderer
        vectorRatio = vec2{window->getWidth() / VECTOR_SCALE.x, window->getHeight() / VECTOR_SCALE.y};

        // Initialize the Jolt Physics system
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        job_system     = std::make_unique<JPH::JobSystemThreadPool>(
                JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);
        constexpr uint32_t cMaxBodies             = 1024;
        constexpr uint32_t cNumBodyMutexes        = 0;
        constexpr uint32_t cMaxBodyPairs          = 1024;
        constexpr uint32_t cMaxContactConstraints = 1024;
        physicsSystem.Init(cMaxBodies,
                           cNumBodyMutexes,
                           cMaxBodyPairs,
                           cMaxContactConstraints,
                           broad_phase_layer_interface,
                           object_vs_broadphase_layer_filter,
                           object_vs_object_layer_filter);
        physicsSystem.SetContactListener(&contactListener);

        // Initialize the various renderers
        initRenderingSystem();
        assert(window != nullptr);

        // Register the built-in nodes types
        registerTypes();
    }

    Application::~Application() {
        log("===== END OF LOG =====");
    }

    void Application::_addNode(const shared_ptr<Node> &node) {
        assert(node != nullptr);
        for (auto& frame : frameData) {
            frame.addedNodes.push_back(node);
        }
        if (node->isProcessed()) {
            node->_onEnterScene();
        }
        for (const auto &child : node->_getChildren()) {
            _addNode(child);
        }
        node->_setAddedToScene(true);
    }

    void Application::_removeNode(const shared_ptr<Node> &node) {
        assert(node != nullptr && node->_isAddedToScene());
        for (auto &child : node->_getChildren()) {
            _removeNode(child);
        }
        for (auto& frame : frameData) {
            frame.removedNodes.push_back(node);
        }
        node->_setAddedToScene(false);
        if (node->isProcessed()) {
            node->_onExitScene();
        }
    }

    void Application::activateCamera(const shared_ptr<Camera> &camera) {
        assert(camera != nullptr);
        for (auto& frame : frameData) {
            frame.activeCamera = camera;
        }
    }


    void Application::drawFrame() {
        if (stopped) { return; }
        processDeferredUpdates(currentFrame);

        // https://gafferongames.com/post/fix_your_timestep/
        double newTime =
                std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double frameTime = newTime - currentTime;
        if (frameTime > 0.25)
            frameTime = 0.25; // Note: Max frame time to avoid spiral of death
        currentTime = newTime;
        accumulator += frameTime;
        while (accumulator >= dt) {
            physicsSystem.Update(dt, 1, temp_allocator.get(), job_system.get());
            physicsProcess(rootNode, dt);
            t += dt;
            accumulator -= dt;
        }
        const double alpha = accumulator / dt;
        process(rootNode, static_cast<float>(alpha));
        renderFrame(currentFrame);
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps            = static_cast<uint32_t>(frameCount / elapsedSeconds);
            frameCount     = 0;
            elapsedSeconds = 0;
        }
        currentFrame = (currentFrame + 1) % applicationConfig.framesInFlight;
    }

    void Application::_onInput(InputEvent &inputEvent) {
        if (stopped) return;
        if (windowManager->onInput(inputEvent)) return;
        input(rootNode, inputEvent);
    }

    void Application::setRootNode(const shared_ptr<Node> &node) {
        assert(node != nullptr && node->getParent() == nullptr && !node->_isAddedToScene());
        waitForRenderingSystem();
        _removeNode(rootNode);
        rootNode = node;
        start();
    }

    void Application::start() {
        ready(rootNode);
        _addNode(rootNode);
        stopped = false;
    }

    void Application::add(const shared_ptr<GWindow> &window) const {
        assert(window != nullptr);
        _instance->windowManager->add(window);
    }

    void Application::remove(const shared_ptr<GWindow> &window) const {
        assert(window != nullptr);
        _instance->windowManager->remove(window);
    }

    void Application::ready(const shared_ptr<Node> &node) {
        assert(node != nullptr);
        for (auto &child : node->_getChildren()) {
            ready(child);
        }
        if (node->isProcessed()) {
            node->_onReady();
        }
    }

    bool Application::input(const shared_ptr<Node> &node, InputEvent &inputEvent) {
        assert(node != nullptr);
        if (!node->_isAddedToScene()) {
            return false;
        }
        for (auto &child : node->_getChildren()) {
            if (input(child, inputEvent))
                return true;
        }
        if (node->isProcessed()) {
            return node->onInput(inputEvent);
        }
        return false;
    }

    void Application::process(const shared_ptr<Node> &node, const float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->onProcess(delta);
        }
        for (auto &child : node->_getChildren()) {
            process(child, delta);
        }
    }

    void Application::physicsProcess(const shared_ptr<Node> &node, const float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->_physicsUpdate(delta);
            node->onPhysicsProcess(delta);
        }
        for (auto &child : node->_getChildren()) {
            physicsProcess(child, delta);
        }
    }

    void Application::pause(const shared_ptr<Node> &node) {
        assert(node != nullptr);
        if (paused && (!node->isProcessed())) {
            node->_onPause();
        }
        if ((!paused && (node->isProcessed()))) {
            node->_onResume();
        }
        for (auto &child : node->_getChildren()) {
            pause(child);
        }
    }

    void Application::setPaused(const bool state) {
        paused = state;
        pause(rootNode);
    }

    void Application::cleanup(shared_ptr<Node> &node) {
        assert(node != nullptr);
        for (auto &child : node->_getChildren()) {
            cleanup(child);
        }
        node.reset();
    }

    void Application::quit() const { window->close(); }

    void Application::_mainLoop() {
        Input::_initInput();
        start();
        auto messageLoop = [] {
#ifndef DISABLE_LOG
            Window::_processDeferredLog();
#endif
            Input::_updateInputStates();
#ifdef _WIN32
            MSG _messages;
            while (PeekMessage(&_messages, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&_messages);
                DispatchMessage(&_messages);
            }
#endif
        };
        while (!window->shouldClose()) {
            messageLoop();
            drawFrame();
        }
        stopped = true;
        waitForRenderingSystem();
        Input::_closeInput();
        windowManager.reset();
        cleanup(rootNode);
#ifdef _WIN32
        DestroyWindow(window->_getHandle());
        PostQuitMessage(0);
#endif
    }

    vec3 Application::getGravity() const {
        auto gravity = physicsSystem.GetGravity();
        return vec3{gravity.GetX(), gravity.GetY(), gravity.GetZ()};
    }

    void Application::registerTypes() const {
        TypeRegistry::registerType<Camera>("Camera");
        TypeRegistry::registerType<Character>("Character");
        TypeRegistry::registerType<CollisionArea>("CollisionArea");
        TypeRegistry::registerType<DirectionalLight>("DirectionalLight");
        TypeRegistry::registerType<Environment>("Environment");
        TypeRegistry::registerType<KinematicBody>("KinematicBody");
        TypeRegistry::registerType<Node>("Node");
        TypeRegistry::registerType<OmniLight>("OmniLight");
        TypeRegistry::registerType<RayCast>("RayCast");
        TypeRegistry::registerType<RigidBody>("RigidBody");
        TypeRegistry::registerType<Skybox>("Skybox");
        TypeRegistry::registerType<SpotLight>("SpotLight");
        TypeRegistry::registerType<StaticBody>("StaticBody");
        TypeRegistry::registerType<Viewport>("Viewport");
    }

} // namespace z0
