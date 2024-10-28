module;
#include <cassert>
#include "z0/libraries.h"

module z0;

import :ApplicationConfig;

import :Instance;
import :Device;
import :SceneRenderer;
import :VectorRenderer;
import :TonemappingPostprocessingRenderer;
import :VulkanApplication;

namespace z0 {

    VulkanApplication::VulkanApplication(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        Application{appConfig, node} {
        assert(window != nullptr);
        instance = make_unique<Instance>();
        device = instance->createDevice(applicationConfig, *window);
        init();
    }

    VulkanApplication::~VulkanApplication() {
        OutlineMaterials::_all().clear();
        sceneRenderer.reset();
        vectorRenderer.reset();
        device->cleanup();
        device.reset();
        instance.reset();
    }

    void VulkanApplication::initRenderingSystem() {
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
        device->registerRenderer(tonemappingRenderer);
        device->registerRenderer(sceneRenderer);

        windowManager = make_unique<GManager>(vectorRenderer,
                                              (applicationConfig.appDir / applicationConfig.defaultFontName).string(),
                                              applicationConfig.defaultFontSize);
    }

    void VulkanApplication::processDeferredUpdates(const uint32_t currentFrame) {
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
        if (frameData[currentFrame].activeCamera != nullptr) {
            sceneRenderer->activateCamera(frameData[currentFrame].activeCamera, currentFrame);
            frameData[currentFrame].activeCamera = nullptr;
        }
        sceneRenderer->postUpdateScene(currentFrame);
    }

    void VulkanApplication::setShadowCasting(const bool enable) const {
        sceneRenderer->setShadowCasting(enable);
    }

    float VulkanApplication::getAspectRatio() const { return device->getAspectRatio(); }

    uint64_t VulkanApplication::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string &VulkanApplication::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t VulkanApplication::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }
} // namespace z0
