module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <cassert>
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/RegisterTypes.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>

module Z0;

import :Node;
import :Application;
import :Input;
import :InputEvent;
import :TypeRegistry;
import :DirectionalLight;
import :Environment;
import :Skybox;
import :Window;
import :Device;
import :Material;
import :SceneRenderer;
import :VectorRenderer;
import :GManager;

namespace z0 {

    Application* Application::_instance = nullptr;

    // Redirect the Vulkan validation layers to the logging system
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        log("validation layer: ", pCallbackData->pMessage);
        return VK_FALSE;
    }

    // vkCreateDebugUtilsMessengerEXT linker
    VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger) {
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // vkDestroyDebugUtilsMessengerEXT linker
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator) {
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    Application::Application(const ApplicationConfig& appConfig, const shared_ptr<Node>& node):
        applicationConfig{appConfig},
        rootNode{node} {
        assert(_instance == nullptr);
        assert(rootNode != nullptr);
        _instance = this;

        // https://github.com/zeux/volk
        if (volkInitialize() != VK_SUCCESS) die("Failed to initialize Volk");

        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance

        // Check if all the requested Vulkan layers are supported by the Vulkan instance
        const vector<const char*> requestedLayers = {
#ifndef NDEBUG
            "VK_LAYER_KHRONOS_validation"
#endif
        };
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char* layerName : requestedLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) die("A requested Vulkan layer is not supported");
        }

        vector<const char*> instanceExtensions{};
        // Abstract native platform surface or window objects for use with Vulkan.
        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
        // Provides a mechanism to create a VkSurfaceKHR object (defined by the VK_KHR_surface extension) that refers to a Win32 HWND
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        // for Vulkan Memory Allocator
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifndef NDEBUG
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        // Use Vulkan 1.3.x
        constexpr VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = VK_API_VERSION_1_3
        };
        // Initialize Vulkan instances, extensions & layers
        const VkInstanceCreateInfo createInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            static_cast<uint32_t>(requestedLayers.size()),
            requestedLayers.data(),
            static_cast<uint32_t>(instanceExtensions.size()),
            instanceExtensions.data()
        };
        if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)die("Failed to create Vulkan instance");
        volkLoadInstance(vkInstance);

        // The global display window
        window = make_unique<Window>(applicationConfig);
        vectorRatio = vec2 {
            window->getWidth() / VECTOR_SCALE.x,
            window->getHeight() / VECTOR_SCALE.y
        };
        // The global Vulkan device helper
        device = make_unique<Device>(vkInstance, requestedLayers, applicationConfig, *window);

        // Initialize validating layer for logging
        constexpr VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = nullptr,
        };
        if (CreateDebugUtilsMessengerEXT(vkInstance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            die("failed to set up debug messenger!");
        }

        // Initialize the Jolt Physics system
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        job_system = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs,
                                                                JPH::cMaxPhysicsBarriers,
                                                                JPH::thread::hardware_concurrency() - 1);
        constexpr uint32_t cMaxBodies = 1024;
        constexpr uint32_t cNumBodyMutexes = 0;
        constexpr uint32_t cMaxBodyPairs = 1024;
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
        const string shaderDir{(applicationConfig.appDir / "shaders").string()};
        sceneRenderer = make_shared<SceneRenderer>(*device, shaderDir);
        vectorRenderer = make_shared<VectorRenderer>(*device,
                                                     shaderDir,
                                                     sceneRenderer->getColorAttachment());
        device->registerRenderer(vectorRenderer);
        device->registerRenderer(sceneRenderer);

        // The global UI window manager
        windowManager = make_unique<GManager>(vectorRenderer,
                                              (applicationConfig.appDir / applicationConfig.defaultFontName).string(),
                                              applicationConfig.defaultFontSize);

        registerTypes();
    }

    Application::~Application() {
        OutlineMaterials::_all().clear();
        sceneRenderer.reset();
        vectorRenderer.reset();
        device->cleanup();
#ifndef NDEBUG
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
#endif
        vkDestroyInstance(vkInstance, nullptr);
        log("===== END OF LOG =====");
    }

    void Application::_addNode(const shared_ptr<Node>& node) {
        assert(node != nullptr);
        addedNodes.push_back(node);
        if (node->isProcessed()) { node->_onEnterScene(); }
        for (const auto& child : node->_getChildren()) {
            _addNode(child);
        }
        node->_setAddedToScene(true);
    }

    void Application::_removeNode(const shared_ptr<Node>& node) {
        assert(node != nullptr && node->_isAddedToScene());
        for (auto& child : node->_getChildren()) {
            _removeNode(child);
        }
        removedNodes.push_back(node);
        node->_setAddedToScene(false);
        if (node->isProcessed()) { node->_onExitScene(); }
    }

    void Application::activateCamera(const shared_ptr<Camera>& camera) const {
        assert(camera != nullptr);
        sceneRenderer->activateCamera(camera);
    }

    void Application::drawFrame() {
        if (stopped) return;

        // Process the deferred scene tree modifications
        sceneRenderer->preUpdateScene();
        windowManager->drawFrame();
        if (!removedNodes.empty()) {
            for (const auto& node : removedNodes) {
                sceneRenderer->removeNode(node);
            }
            removedNodes.clear();
            if (sceneRenderer->getCamera() == nullptr) {
                const auto& camera = rootNode->findFirstChild<Camera>(true);
                if (camera->isProcessed()) {
                    sceneRenderer->activateCamera(camera);
                }
            }
        }
        if (!addedNodes.empty()) {
            for (const auto& node : addedNodes) {
                sceneRenderer->addNode(node);
            }
            addedNodes.clear();
        }
        sceneRenderer->postUpdateScene();

        // https://gafferongames.com/post/fix_your_timestep/
        double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).
            count();
        double frameTime = newTime - currentTime;
        if (frameTime > 0.25) frameTime = 0.25; // Note: Max frame time to avoid spiral of death
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
        device->drawFrame();
        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 1.0) {
            fps = static_cast<uint32_t>(frameCount / elapsedSeconds);
            frameCount = 0;
            elapsedSeconds = 0;
        }
    }

    void Application::_onInput(InputEvent& inputEvent) {
        if (stopped) return;
        if (windowManager->onInput(inputEvent)) { return; }
        input(rootNode, inputEvent);
    }

    void Application::setRootNode(const shared_ptr<Node>& node) {
        assert(node != nullptr && node->getParent() == nullptr && !node->_isAddedToScene());
        device->wait();
        _removeNode(rootNode);
        rootNode = node;
        start();
    }

    void Application::start() {
        ready(rootNode);
        _addNode(rootNode);
        stopped = false;
    }

    void Application::addWindow(const shared_ptr<GWindow>& window) const {
        assert(window != nullptr);
        _instance->windowManager->add(window);
    }

    void Application::removeWindow(const shared_ptr<GWindow>& window) const {
        assert(window != nullptr);
        _instance->windowManager->remove(window);
    }

    void Application::ready(const shared_ptr<Node>& node) {
        assert(node != nullptr);
        for (auto& child : node->_getChildren()) {
            ready(child);
        }
        if (node->isProcessed()) {
            node->_onReady();
        }
    }

    bool Application::input(const shared_ptr<Node>& node, InputEvent& inputEvent) {
        assert(node != nullptr);
        if (!node->_isAddedToScene()) { return false; }
        for (auto& child : node->_getChildren()) {
            if (input(child, inputEvent)) return true;
        }
        if (node->isProcessed()) {
            return node->onInput(inputEvent);
        }
        return false;
    }

    void Application::process(const shared_ptr<Node>& node, const float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->onProcess(delta);
        }
        for (auto& child : node->_getChildren()) {
            process(child, delta);
        }
    }

    void Application::physicsProcess(const shared_ptr<Node>& node, const float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->_physicsUpdate(delta);
            node->onPhysicsProcess(delta);
        }
        for (auto& child : node->_getChildren()) {
            physicsProcess(child, delta);
        }
    }

    void Application::pause(const shared_ptr<Node>& node) {
        assert(node != nullptr);
        if (paused && (!node->isProcessed())) {
            node->_onPause();
        }
        if ((!paused && (node->isProcessed()))) {
            node->_onResume();
        }
        for (auto& child : node->_getChildren()) {
            pause(child);
        }
    }

    void Application::setPaused(const bool state) {
        paused = state;
        pause(rootNode);
    }

    void Application::cleanup(shared_ptr<Node>& node) {
        assert(node != nullptr);
        for (auto& child : node->_getChildren()) {
            cleanup(child);
        }
        node.reset();
    }

    void Application::quit() const {
        window->close();
    }

    void Application::_mainLoop() {
        Input::_initInput();
        start();
        while (!window->shouldClose()) {
#ifndef DISABLE_LOG
            Window::_processDeferredLog();
#endif
            Input::_updateInputStates();
#ifdef _WIN32
            MSG _messages;
            if (PeekMessage(&_messages, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&_messages);
                DispatchMessage(&_messages);
            }
#endif
            drawFrame();
        }
        stopped = true;
        Input::_closeInput();
        device->wait();
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
        TypeRegistry::registerType<Node>("Node");
        TypeRegistry::registerType<DirectionalLight>("DirectionalLight");
        TypeRegistry::registerType<Environment>("Environment");
        TypeRegistry::registerType<Skybox>("Skybox");
    }

    void Application::setShadowCasting(const bool enable) const {
        sceneRenderer->setShadowCasting(enable);
    }

    uint64_t Application::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string& Application::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t Application::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }
}
