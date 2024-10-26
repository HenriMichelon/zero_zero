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

module z0;

import :Node;
import :Application;
import :Input;
import :InputEvent;
import :TypeRegistry;
import :Window;
import :Device;
import :Material;
import :SceneRenderer;
import :VectorRenderer;
import :SimplePostprocessingRenderer;
import :TonemappingPostprocessingRenderer;
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

#ifndef NDEBUG
    // Redirect the Vulkan validation layers to the logging system
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                        VkDebugUtilsMessageTypeFlagsEXT,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *) {
        log("validation layer: ", pCallbackData->pMessage);
        return VK_FALSE;
    }

    // vkCreateDebugUtilsMessengerEXT linker
    VkResult CreateDebugUtilsMessengerEXT(const VkInstance                          instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks              *pAllocator,
                                          VkDebugUtilsMessengerEXT                 *pDebugMessenger) {
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // vkDestroyDebugUtilsMessengerEXT linker
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator) {
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

#endif

    Application::Application(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        applicationConfig{appConfig}, rootNode{node} {
        assert(_instance == nullptr);
        assert(rootNode != nullptr);
        _instance = this;
        frameData.resize(applicationConfig.framesInFlight);

        // https://github.com/zeux/volk
        if (volkInitialize() != VK_SUCCESS) { die("Failed to initialize Volk"); }

        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance

#ifndef NDEBUG
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
#endif

        // Check if all the requested Vulkan layers are supported by the Vulkan instance
        const vector<const char *> requestedLayers = {
#ifndef NDEBUG
            validationLayerName
#endif
    };
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char *layerName : requestedLayers) {
            bool layerFound = false;
            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
                die("A requested Vulkan layer is not supported");
        }

        vector<const char *> instanceExtensions{};
        // Abstract native platform surface or window objects for use with Vulkan.
        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
        // Provides a mechanism to create a VkSurfaceKHR object (defined by the VK_KHR_surface extension)
        // that refers to a Win32 HWND
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        // For Vulkan Memory Allocator
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifndef NDEBUG
        // To use a debug callback for validation message
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // https://docs.vulkan.org/samples/latest/samples/extensions/shader_debugprintf/README.html
        // Use run/debug config with environment variables (cf. shader_debug_env.cmd)
        /*instanceExtensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
        const char *setting_debug_action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
        const char *setting_validate_gpu_based[] = {"GPU_BASED_DEBUG_PRINTF"};
        constexpr VkBool32 setting_enable_printf_to_stdout{VK_TRUE};

        const auto layerSettings = array{
                VkLayerSettingEXT{validationLayerName, "debug_action",
                    VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_debug_action},
                VkLayerSettingEXT{validationLayerName, "validate_gpu_based",
                    VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_validate_gpu_based},
                VkLayerSettingEXT{validationLayerName, "printf_to_stdout",
                    VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_printf_to_stdout},
        };
        const auto layerSettingsCreateInfo = VkLayerSettingsCreateInfoEXT {
            .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
            .pNext = nullptr,
            .settingCount = layerSettings.size(),
            .pSettings = layerSettings.data()
        };*/

#endif

        // Use Vulkan 1.3.x
        constexpr VkApplicationInfo applicationInfo{.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                                    .apiVersion = VK_API_VERSION_1_3};
        // Initialize Vulkan instances, extensions & layers
        const VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                 nullptr,
                                                 0,
                                                 &applicationInfo,
                                                 static_cast<uint32_t>(requestedLayers.size()),
                                                 requestedLayers.data(),
                                                 static_cast<uint32_t>(instanceExtensions.size()),
                                                 instanceExtensions.data()};
        if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
            die("Failed to create Vulkan instance");
        volkLoadInstance(vkInstance);

        // The rendering window
        window = make_unique<Window>(applicationConfig);
        // Compute the scale ratios for the vector renderer
        vectorRatio = vec2{window->getWidth() / VECTOR_SCALE.x, window->getHeight() / VECTOR_SCALE.y};
        // The global Vulkan device helper
        device = make_unique<Device>(vkInstance, requestedLayers, applicationConfig, *window);

#ifndef NDEBUG
        // Initialize validating layer for logging
        constexpr VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
                .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
                .pUserData       = nullptr,
        };
        if (CreateDebugUtilsMessengerEXT(vkInstance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            die("failed to set up debug messenger!");
        }
#endif

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
        const string shaderDir{(applicationConfig.appDir / "shaders").string()};
        sceneRenderer = make_shared<SceneRenderer>(
            *device,
            shaderDir,
            applicationConfig.clearColor);
        tonemappingRenderer = make_shared<TonemappingPostprocessingRenderer>(
            *device,
            shaderDir,
            sceneRenderer->getColorAttachments(),
            sceneRenderer->getDepthAttachments());
        vectorRenderer = make_shared<VectorRenderer>(
            *device,
            shaderDir,
            sceneRenderer->getColorAttachments());

        device->registerRenderer(vectorRenderer);
        // device->registerRenderer(tonemappingRenderer);
        device->registerRenderer(sceneRenderer);

        // The global UI window manager
        windowManager = make_unique<GManager>(vectorRenderer,
                                              (applicationConfig.appDir / applicationConfig.defaultFontName).string(),
                                              applicationConfig.defaultFontSize);

        // Register the built-in nodes types
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
            frame.activateCamera = camera;
        }
    }

    void Application::processDeferredUpdates(const uint32_t currentFrame) {
        // Process the deferred scene tree modifications
        sceneRenderer->preUpdateScene(currentFrame);
        windowManager->drawFrame();
        if (!frameData[currentFrame].removedNodes.empty()) {
            for (const auto &node : frameData[currentFrame].removedNodes) {
                sceneRenderer->removeNode(node, currentFrame);
            }
            frameData[currentFrame].removedNodes.clear();
            if (sceneRenderer->getCamera(currentFrame) == nullptr) {
                const auto &camera = rootNode->findFirstChild<Camera>(true);
                if (camera->isProcessed()) {
                    sceneRenderer->activateCamera(camera, currentFrame);
                }
            }
        }
        if (!frameData[currentFrame].addedNodes.empty()) {
            for (const auto &node : frameData[currentFrame].addedNodes) {
                sceneRenderer->addNode(node, currentFrame);
            }
            frameData[currentFrame].addedNodes.clear();
        }
        if (frameData[currentFrame].activateCamera != nullptr) {
            sceneRenderer->activateCamera(frameData[currentFrame].activateCamera.get(), currentFrame);
            frameData[currentFrame].activateCamera = nullptr;
        }
        sceneRenderer->postUpdateScene(currentFrame);
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
        device->drawFrame(currentFrame);
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
        if (stopped)
            return;
        if (windowManager->onInput(inputEvent)) {
            return;
        }
        input(rootNode, inputEvent);
    }

    void Application::setRootNode(const shared_ptr<Node> &node) {
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
        device->wait();
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

    void Application::setShadowCasting(const bool enable) const { sceneRenderer->setShadowCasting(enable); }

    uint64_t Application::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string &Application::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t Application::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }
} // namespace z0
