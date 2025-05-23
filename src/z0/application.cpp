/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>
#include "z0/libraries.h"

module z0.Application;

import z0.ApplicationConfig;
import z0.Constants;
import z0.Input;
import z0.InputEvent;
import z0.Loader;
import z0.Log;
import z0.Tools;
import z0.TypeRegistry;
import z0.Window;

import z0.nodes.Camera;
import z0.nodes.CollisionArea;
import z0.nodes.DirectionalLight;
import z0.nodes.Environment;
import z0.nodes.Node;
import z0.nodes.OmniLight;
import z0.nodes.RayCast;
import z0.nodes.RigidBody;
import z0.nodes.Skybox;
import z0.nodes.SpotLight;
import z0.nodes.StaticBody;
import z0.nodes.Viewport;

import z0.resources.Material;

import z0.ui.Manager;

namespace z0 {

    // Unique application instance
    Application *Application::_instance = nullptr;

    Application::Application(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        object_vs_object_layer_filter{appConfig.layerCollisionTable.layersCount},
        applicationConfig{appConfig},
        rootNode{node},
        displayDebug{appConfig.debugConfig.displayAtStartup} {
        assert(_instance == nullptr);
        _instance = this;
#ifndef DISABLE_LOG
        log = make_shared<Log>();
        Log::open(log);
#endif

        frameData.resize(applicationConfig.framesInFlight);
        // The rendering Window
        if (node != nullptr) {
            window = make_unique<Window>(applicationConfig);
        }
        // The layer vs layer collision table initialization
        for (const auto &layerCollide : appConfig.layerCollisionTable.layersCollideWith) {
            for (const auto &layer : layerCollide.collideWith) {
                object_vs_object_layer_filter.EnableCollision(layerCollide.layer, layer);
            }
        }
    }

    void Application::init() {
        TRACE();

        // Compute the scale ratios for the vector renderer
        vectorExtent = vec2{(window->getWidth() * 1000.0f) / window->getHeight(), 1000.0f};
        vectorRatio = vectorExtent.x / vectorExtent.y;
        // vectorRatio = vec2{window->getWidth() / vectorExtent.x, window->getHeight() / vectorExtent.y};

        // Initialize the Jolt Physics system
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        temp_allocator = make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        job_system     = make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers);
        physicsSystem.Init(1024,
                           0,
                           2048,
                           104,
                           broad_phase_layer_interface,
                           object_vs_broadphase_layer_filter,
                           object_vs_object_layer_filter);
        physicsSystem.SetContactListener(&contactListener);

        // Initialize the various renderers
        initRenderingSystem();
        assert(window != nullptr);

        outlineMaterials = make_unique<OutlineMaterials>();

        // Register the built-in nodes types
        registerTypes();
    }

    Application::~Application() {
        TRACE();
#ifndef DISABLE_LOG
        Log::close();
#endif
    }

    void Application::_addNode(const shared_ptr<Node> &node, const bool async) {
        TRACE();
        assert(node != nullptr);
        _lockDeferredUpdate();
        {
            auto lock = lock_guard(frameDataMutex);
            for (auto& frame : frameData) {
                if (async) {
                    frame.addedNodesAsync.push_back(node );
                } else {
                    frame.addedNodes.push_back(node );
                }
            }
        }
        node->_onEnterScene();
        for (const auto &child : node->_getChildren()) {
            _addNode(child, async);
        }
        node->_setAddedToScene(true);
        _unlockDeferredUpdate();
    }

    void Application::_removeNode(const shared_ptr<Node> &node, const bool async) {
        TRACE();
        assert(node != nullptr && node->_isAddedToScene());
        if (!stopped) {
            _lockDeferredUpdate();
            for (auto &child : node->_getChildren()) {
                _removeNode(child, async);
            }
            {
                auto lock = lock_guard(frameDataMutex);
                for (auto& frame : frameData) {
                    if (async) {
                        frame.removedNodesAsync.push_back(node);
                    } else {
                        frame.removedNodes.push_back(node);
                    }
                }
            }
        }
        node->_setAddedToScene(false);
        node->_onExitScene();
        _unlockDeferredUpdate();
    }

    void Application::activateCamera(const shared_ptr<Camera> &camera) {
        TRACE();
        _lockDeferredUpdate();
        for (auto& frame : frameData) {
            frame.activeCamera = camera;
            frame.cameraChanged = true;
        }
        _unlockDeferredUpdate();
    }

    void Application::_lockDeferredUpdate() {
        doDeferredUpdates = false;
    }

    void Application::_unlockDeferredUpdate() {
        doDeferredUpdates = true;
    }

    void Application::drawFrame() {
        if (stopped) { return; }
        // TRACE();
        // Add/removes nodes from the scene
        if (doDeferredUpdates) {
            processDeferredUpdates(currentFrame);
        }
        // Physics events & others deferred calls
        if (!deferredCalls.empty()) {
            ranges::for_each(deferredCalls, [](const function<void()> &call) { call(); });
            auto lock = lock_guard(deferredCallsMutex);
            deferredCalls.clear();
        }
        // Clean up the async calls
        if (!threadedCalls.empty()) {
            auto lock = lock_guard(threadedCallsMutex);
            for (auto it = threadedCalls.begin(); it != threadedCalls.end();) {
                if (it->joinable()) {
                    ++it;
                } else {
                    it = threadedCalls.erase(it);
                }
            }
        }

        // https://gafferongames.com/post/fix_your_timestep/
        const double newTime = chrono::duration_cast<chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double frameTime = newTime - currentTime;

        // Calculate the FPS
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps            = static_cast<uint32_t>(frameCount / elapsedSeconds);
            frameCount     = 0;
            elapsedSeconds = 0;
        }

        if (frameTime > 0.25) {
            frameTime = 0.25; // Note: Max frame time to avoid spiral of death
        }
        currentTime = newTime;
        accumulator += frameTime;
        {
            auto lock = lock_guard(rootNodeMutex);
            while (accumulator >= dt) {
                physicsSystem.Update(dt, 1, temp_allocator.get(), job_system.get());
                physicsProcess(rootNode, dt);
                t += dt;
                accumulator -= dt;
            }
            process(rootNode, static_cast<float>(accumulator / dt));
        }
        renderFrame(currentFrame);
        currentFrame = (currentFrame + 1) % applicationConfig.framesInFlight;
    }

    void Application::_onInput(InputEvent &inputEvent) {
        // TRACE();
        if (stopped) return;
        if (windowManager->onInput(inputEvent)) return;
        input(rootNode, inputEvent);
    }

    void Application::setRootNode(const shared_ptr<Node> &node) {
        TRACE();
        assert(node != nullptr && node->getParent() == nullptr && !node->_isAddedToScene());
        // waitForRenderingSystem();
        {
            auto lock = lock_guard(rootNodeMutex);
            _removeNode(rootNode, false);
            rootNode = node;
        }
        start();
    }

    void Application::start() {
        TRACE();
        ready(rootNode);
        _addNode(rootNode, false);
        stopped = false;
    }

    shared_ptr<ui::Window> Application::add(const shared_ptr<ui::Window> &window) {
        TRACE();
        assert(window != nullptr);
        return _instance->windowManager->add(window);
    }

    void Application::remove(const shared_ptr<ui::Window> &window) const {
        TRACE();
        assert(window != nullptr);
        _instance->windowManager->remove(window);
    }

    void Application::ready(const shared_ptr<Node> &node) {
        TRACE();
        assert(node != nullptr);
        for (auto &child : node->_getChildren()) {
            ready(child);
        }
        if (node->isProcessed()) {
            node->_onReady();
        }
    }

    bool Application::input(const shared_ptr<Node> &node, InputEvent &inputEvent) {
        // TRACE();
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

    // void Application::callDeferred(std::function<void()> func) {
    //     deferredCalls.push_back(func);
    // }

    void Application::process(const shared_ptr<Node> &node, const float alpha) {
        // TRACE();
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->_update(alpha);
            node->onProcess(alpha);
        }
        for (auto &child : node->_getChildren()) {
            process(child, alpha);
        }
    }

    void Application::physicsProcess(const shared_ptr<Node> &node, const float delta) {
        // TRACE();
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
        TRACE();
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
        TRACE();
        assert(node != nullptr);
        _removeNode(node, false);
        for (auto &child : node->_getChildren()) {
            cleanup(child);
        }
        node.reset();
    }

    void Application::quit() const {
        TRACE();
        window->close();
    }

    void Application::_mainLoop() {
        TRACE();
        Input::_initInput();
        start();
        auto messageLoop = [] {
            Input::_updateInputStates();
#ifdef _WIN32
            MSG _messages;
            while (PeekMessage(&_messages, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&_messages);
                DispatchMessage(&_messages);
            }
#endif
        };
        currentTime = chrono::duration_cast<chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        while (!window->shouldClose()) {
            messageLoop();
            drawFrame();
        }
        stopped = true;
        stopRenderingSystem();
        for(auto&t : threadedCalls) {
            t.join();
        }
        Input::_closeInput();
        Loader::clearCache();
        cleanup(rootNode);
        windowManager.reset();
        rootNode.reset();
        waitForRenderingSystem();
#ifdef _WIN32
        DestroyWindow(window->_getHandle());
        PostQuitMessage(0);
#endif
        TRACE();
    }

    vec3 Application::getGravity() const {
        const auto gravity = physicsSystem.GetGravity();
        return vec3{gravity.GetX(), gravity.GetY(), gravity.GetZ()};
    }

    void Application::setDisplayDebug(bool display) {
        callDeferred([this, display]{ displayDebug = display; });
    }

    void Application::registerTypes() const {
        TypeRegistry::registerType<Camera>("Camera");
        TypeRegistry::registerType<CollisionArea>("CollisionArea");
        TypeRegistry::registerType<DirectionalLight>("DirectionalLight");
        TypeRegistry::registerType<Environment>("Environment");
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
