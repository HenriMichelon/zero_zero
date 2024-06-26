#include "z0/z0.h"
#ifndef USE_PCH
#include "z0/input.h"
#include "z0/resources/image.h"
#include "z0/resources/texture.h"
#include "z0/resources/material.h"
#include "z0/resources/mesh.h"
#include "z0/resources/font.h"
#include "z0/resources/cubemap.h"
#include "z0/nodes/node.h"
#include "z0/nodes/camera.h"
#include "z0/nodes/environment.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/nodes/light.h"
#include "z0/nodes/directional_light.h"
#include "z0/nodes/omni_light.h"
#include "z0/framebuffers/shadow_map_frame_buffer.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/renderers/vector_renderer.h"
#include "z0/renderers/skybox_renderer.h"
#include "z0/renderers/base_models_renderer.h"
#include "z0/renderers/shadowmap_renderer.h"
#include "z0/renderers/base_postprocessing_renderer.h"
#include "z0/renderers/simple_postprocessing_renderer.h"
#include "z0/renderers/scene_renderer.h"
#include "z0/gui/gresource.h"
#include "z0/gui/gstyle.h"
#include "z0/gui/gevent.h"
#include "z0/gui/gwindow.h"
#include "z0/gui/gmanager.h"
#include "z0/application.h"
#endif

#include <Jolt/RegisterTypes.h>

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
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    // vkDestroyDebugUtilsMessengerEXT linker
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    Application::Application(const ApplicationConfig &appConfig, const shared_ptr<Node>& node):
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
        const VkApplicationInfo applicationInfo{
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
        // The global Vulkan device helper
        device = make_unique<Device>(vkInstance, requestedLayers, applicationConfig, *window);

        // Initialize validating layer loggin
        const VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
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
        const uint32_t cMaxBodies = 1024;
        const uint32_t cNumBodyMutexes = 0;
        const uint32_t cMaxBodyPairs = 1024;
        const uint32_t cMaxContactConstraints = 1024;
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
        device->cleanup();
#ifndef NDEBUG
        DestroyDebugUtilsMessengerEXT(vkInstance, debugMessenger, nullptr);
#endif
        vkDestroyInstance(vkInstance, nullptr);
    }

    void Application::_addNode(const shared_ptr<Node>& node) {
        assert(node != nullptr);
        addedNodes.push_back(node);
        node->_onEnterScene();
        for(const auto& child: node->_getChildren()) {
            _addNode(child);
        }
        node->_setAddedToScene(true);
    }

    void Application::_removeNode(const shared_ptr<Node> &node) {
        assert(node != nullptr && node->_isAddedToScene());
        for(auto& child: node->_getChildren()) {
            _removeNode(child);
        }
        removedNodes.push_back(node);
        node->_setAddedToScene(false);
        node->_onExitScene();
    }

    void Application::activateCamera(const shared_ptr<Camera> &camera) {
        assert(camera != nullptr);
        if (rootNode->haveChild(camera, true)) {
            sceneRenderer->activateCamera(camera);
        }
    }

    void Application::drawFrame() {
        if (stopped) return;

        // Process the deferred scene tree modification
        sceneRenderer->preUpdateScene();
        windowManager->drawFrame();
        if (!removedNodes.empty()) {
            for (const auto &node: removedNodes) {
                sceneRenderer->removeNode(node);
            }
            removedNodes.clear();
            if (sceneRenderer->getCamera() == nullptr) {
                sceneRenderer->activateCamera(rootNode->findFirstChild<Camera>(true));
            }
        }
        if (!addedNodes.empty()) {
            for (const auto &node: addedNodes) {
                sceneRenderer->addNode(node);
            }
            addedNodes.clear();
        }
        sceneRenderer->postUpdateScene();

        // https://gafferongames.com/post/fix_your_timestep/
        double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
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
#ifdef VULKAN_STATS
            VulkanStats::get().averageFps = static_cast<uint32_t >((static_cast<float>(VulkanStats::get().averageFps) + fps) / 2.0f);
#endif
            frameCount = 0;
            elapsedSeconds = 0;
        }
    }

    void Application::_onInput(InputEvent &inputEvent) {
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

    void Application::addWindow(const shared_ptr<GWindow>& window) {
        assert(window != nullptr);
        _instance->windowManager->add(window);
    }

    void Application::removeWindow(const shared_ptr<GWindow>& window) {
        assert(window != nullptr);
        _instance->windowManager->remove(window);
    }

    void Application::ready(const shared_ptr<Node>& node) {
        assert(node != nullptr);
        for(auto& child: node->_getChildren()) {
            ready(child);
        }
        node->_onReady();
    }

    bool Application::input(const shared_ptr<Node>& node, InputEvent& inputEvent) {
        assert(node != nullptr);
        if (!node->_isAddedToScene()) { return false; }
        for(auto& child: node->_getChildren()) {
            if (input(child, inputEvent)) return true;
        }
        if (node->isProcessed()) {
            return node->onInput(inputEvent);
        } else {
            return false;
        }
    }

    void Application::process(const shared_ptr<Node>& node, float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->onProcess(delta);
        }
        for(auto& child: node->_getChildren()) {
            process(child, delta);
        }
    }

    void Application::physicsProcess(const shared_ptr<Node>& node, float delta) {
        assert(node != nullptr);
        if (node->isProcessed()) {
            node->_physicsUpdate(delta);
            node->onPhysicsProcess(delta);
        }
        for(auto& child: node->_getChildren()) {
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
        for(auto& child: node->_getChildren()) {
            pause(child);
        }
    }

    void  Application::setPaused(bool state) { 
        paused = state; 
        pause(rootNode);
    }

    void Application::cleanup(shared_ptr<z0::Node> &node) {
        assert(node != nullptr);
        for(auto& child: node->_getChildren()) {
            cleanup(child);
        }
        node.reset();
    }

    void Application::quit() {
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
#ifdef VULKAN_STATS
        VulkanStats::get().display();
#endif
    }

    const Window &Application::getWindow() const {
        return *window;
    }

    Application &Application::get() {
        assert(_instance != nullptr);
        return *_instance;
    }
    
    const ApplicationConfig& Application::getConfig() const { 
        return applicationConfig; 
    }

    vec3 Application::getGravity() const {
        auto gravity = physicsSystem.GetGravity();
        return vec3{gravity.GetX(), gravity.GetY(), gravity.GetZ()};
    }

    void Application::registerTypes() {
        TypeRegistry::registerType<Node>("Node");
        TypeRegistry::registerType<DirectionalLight>("DirectionalLight");
        TypeRegistry::registerType<Environment>("Environment");
    }

}
