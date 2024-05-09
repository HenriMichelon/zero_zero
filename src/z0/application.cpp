#include "z0/application.h"
#include "z0/window.h"
#include "z0/stats.h"
#include "z0/input.h"

#include <vulkan/vulkan.hpp>

#include <Jolt/RegisterTypes.h>

#include <cassert>

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
    if (z0::Application::_instance == nullptr) z0::die("No Application object found");
    z0::Application& application = z0::Application::get();
    application._mainLoop();
    return 0;
}
#endif

namespace z0 {

    Application* Application::_instance = nullptr;

    Application::Application(const z0::ApplicationConfig &appConfig, const shared_ptr<Node>& node):
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

        window = make_unique<Window>(applicationConfig);
        device = make_unique<Device>(vkInstance, requestedLayers, applicationConfig, *window);

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

        const string shaderDir{(applicationConfig.appDir / "shaders").string()};
        sceneRenderer = make_shared<SceneRenderer>(*device, shaderDir);
        vectorRenderer = make_shared<VectorRenderer>(*device,
                                                     shaderDir,
                                                     sceneRenderer->getColorAttachement());
        device->registerRenderer(vectorRenderer);
        device->registerRenderer(sceneRenderer);
        windowManager = make_unique<GManager>(vectorRenderer);
    }

    Application::~Application() {
        device->cleanup();
        vkDestroyInstance(vkInstance, nullptr);
    }

    void Application::addNode(const shared_ptr<z0::Node>& node) {
        addedNodes.push_back(node);
        node->_setAddedToScene(true);
        node->_onEnterScene();
        for(const auto& child: node->getChildren()) {
            addNode(child);
        }
    }

    void Application::removeNode(const shared_ptr<z0::Node> &node) {
        for(auto& child: node->getChildren()) {
            removeNode(child);
        }
        removedNodes.push_back(node);
        node->_setAddedToScene(false);
        node->_onExitScene();
    }

    void Application::activateCamera(const shared_ptr<z0::Camera> &camera) {
        if (rootNode->haveChild(camera, true)) {
            sceneRenderer->activateCamera(camera);
        }
    }

    void Application::drawFrame() {
        if (stopped) return;

        if (!addedNodes.empty()) {
            for (const auto &node: addedNodes) {
                sceneRenderer->addNode(node);
            }
            addedNodes.clear();
        }
        if (!removedNodes.empty()) {
            for (const auto &node: removedNodes) {
                sceneRenderer->removeNode(node);
            }
            removedNodes.clear();
        }

        // https://gafferongames.com/post/fix_your_timestep/
        double newTime = std::chrono::duration_cast<std::chrono::duration<double>>(Clock::now().time_since_epoch()).count();
        double frameTime = newTime - currentTime;
        if (frameTime > 0.25) frameTime = 0.25; // Note: Max frame time to avoid spiral of death
        currentTime = newTime;
        accumulator += frameTime;
        while (accumulator >= dt) {
            physicsProcess(rootNode, dt);
            physicsSystem.Update(dt, 1, temp_allocator.get(), job_system.get());
            t += dt;
            accumulator -= dt;
        }

        const double alpha = accumulator / dt;
        process(rootNode, static_cast<float>(alpha));
        device->drawFrame();

        elapsedSeconds += static_cast<float>(frameTime);
        frameCount++;
        if (elapsedSeconds >= 0.250) {
            auto fps = static_cast<float>(frameCount) / elapsedSeconds;
#ifdef VULKAN_STATS
            VulkanStats::get().averageFps = static_cast<uint32_t >((static_cast<float>(VulkanStats::get().averageFps) + fps) / 2.0f);
#endif
            //viewport->_setFPS(fps);
            frameCount = 0;
            elapsedSeconds = 0;
        }
    }

    void Application::onInput(z0::InputEvent &inputEvent) {
        input(rootNode, inputEvent);
    }

    void Application::start() {
        ready(rootNode);
        addNode(rootNode);
        windowManager->refresh();
        /*vectorRenderer->beginDraw();
        vectorRenderer->setPenColor({1.0, 0.647, 0.0});
        vectorRenderer->setTransparency(1.0);
        vectorRenderer->drawFilledRect({0.25, 0.975}, { 0.75, 0.95});
        vectorRenderer->setTransparency(0.1);
        vectorRenderer->drawFilledRect({0.25, 0.75}, { 0.75, 0.25});
        vectorRenderer->endDraw();*/
    }

    void Application::end() {
        cleanup(rootNode);
#ifdef VULKAN_STATS
        VulkanStats::get().display();
#endif
    }

    void Application::add(shared_ptr<z0::GWindow> &window) {
        _instance->windowManager->add(window);
    }

    void Application::ready(const std::shared_ptr<Node>& node) {
        for(auto& child: node->getChildren()) {
            ready(child);
        }
        node->_onReady();
    }

    bool Application::input(const std::shared_ptr<Node>& node, InputEvent& inputEvent) {
        for(auto& child: node->getChildren()) {
            if (input(child, inputEvent)) return true;
        }
        return node->onInput(inputEvent);
    }

    void Application::process(const std::shared_ptr<Node>& node, float delta) {
        if (node->isProcessed()) {
            node->onProcess(delta);
        }
        for(auto& child: node->getChildren()) {
            process(child, delta);
        }
    }
    void Application::physicsProcess(const std::shared_ptr<Node>& node, float delta) {
        if (node->isProcessed()) {
            if (node->_needPhysics()) node->_physicsUpdate();
            node->onPhysicsProcess(delta);
        }
        for(auto& child: node->getChildren()) {
            physicsProcess(child, delta);
        }
    }

    void Application::cleanup(shared_ptr<z0::Node> &node) {
        for(auto& child: node->getChildren()) {
            cleanup(child);
        }
        node.reset();
    }

#ifdef _WIN32
    void Application::_mainLoop() {
        start();
        while (!window->shouldClose()) {
            MSG _messages;
            if (PeekMessage(&_messages, nullptr, 0, 0, PM_REMOVE)) {
                TranslateMessage(&_messages);
                DispatchMessage(&_messages);
            }
            drawFrame();
        }
        device->wait();
        DestroyWindow(window->_getHandle());
        PostQuitMessage(0);
        end();
    }
#endif

    const Window &Application::getWindow() const {
        return *window;
    }

    Application &Application::get() {
        assert(_instance != nullptr);
        return *_instance;
    }

}
