/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cassert>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include "z0/libraries.h"

module z0.VulkanApplication;

import z0.ApplicationConfig;
import z0.Camera;
import z0.Material;
import z0.Node;
import z0.Tools;

import z0.GManager;

import z0.Instance;
import z0.Device;
import z0.SceneRenderer;
import z0.VectorRenderer;
import z0.TonemappingPostprocessingRenderer;
import z0.VulkanImage;

namespace z0 {

    VulkanApplication::VulkanApplication(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        Application{appConfig, node} {
        assert(window != nullptr);
        instance = make_unique<Instance>();
        device = instance->createDevice(applicationConfig, *window);
        // KTXVulkanImage::initialize(
        //     device->getPhysicalDevice(), device->getDevice(),
        //     device->getGraphicQueue(), device->getCommandPool());
        const auto& conf = appConfig.debugPhysicsConfig;
        bodyDrawSettings = JPH::BodyManager::DrawSettings{
            .mDrawGetSupportFunction = conf.drawGetSupportingFace,
            .mDrawShape = conf.drawShape,
            .mDrawShapeWireframe = true,
            .mDrawShapeColor = static_cast<JPH::BodyManager::EShapeColor>(conf.drawShapeColor),
            .mDrawBoundingBox = conf.drawBoundingBox,
            .mDrawVelocity = conf.drawVelocity,
            .mDrawMassAndInertia = conf.drawMassAndInertia,
            .mDrawSleepStats = conf.drawSleepStats,
        };
        init();
    }

    VulkanApplication::~VulkanApplication() {
        sceneRenderer.reset();
        if (vectorRenderer) { vectorRenderer.reset(); }
        if (tonemappingRenderer) { tonemappingRenderer.reset(); }
        if (debugCollisionObjectsRenderer) { debugCollisionObjectsRenderer.reset(); }
        // KTXVulkanImage::cleanup();
        device->cleanup();
        device.reset();
        instance.reset();
    }

    void VulkanApplication::initRenderingSystem() {
        sceneRenderer = make_shared<SceneRenderer>(
            *device,
            applicationConfig.clearColor,
            applicationConfig.useDepthPrepass,
            applicationConfig.debugPhysics);
        tonemappingRenderer = make_shared<TonemappingPostprocessingRenderer>(
        *device,
        sceneRenderer->getColorAttachments(),
        sceneRenderer->getDepthAttachments());
        vectorRenderer = make_shared<VectorRenderer>(
            *device,
            sceneRenderer->getColorAttachments());

        device->registerRenderer(vectorRenderer);
        device->registerRenderer(tonemappingRenderer);
        if (applicationConfig.debugPhysics) {
            debugCollisionObjectsRenderer = make_shared<DebugCollisionObjectsRenderer>(
                *device,
                sceneRenderer->getColorAttachments(),
                sceneRenderer->getDepthAttachments());
            device->registerRenderer(debugCollisionObjectsRenderer);
        }
        device->registerRenderer(sceneRenderer);

        windowManager = make_unique<GManager>(vectorRenderer,
                                              applicationConfig.defaultFontName,
                                              applicationConfig.defaultFontSize);
    }

    void VulkanApplication::processDeferredUpdates(const uint32_t currentFrame) {
        sceneRenderer->preUpdateScene(currentFrame);
        windowManager->drawFrame();
        if (!frameData.at(currentFrame).removedNodes.empty()) {
            for (const auto &node : frameData.at(currentFrame).removedNodes) {
                sceneRenderer->removeNode(node, currentFrame);
            }
            frameData.at(currentFrame).removedNodes.clear();
        }
        if (!frameData.at(currentFrame).addedNodes.empty()) {
            for (const auto &node : frameData.at(currentFrame).addedNodes) {
                sceneRenderer->addNode(node, currentFrame);
            }
            frameData.at(currentFrame).addedNodes.clear();
        }
        if (frameData.at(currentFrame).activeCamera != nullptr) {
            sceneRenderer->activateCamera(frameData.at(currentFrame).activeCamera, currentFrame);
            if (applicationConfig.debugPhysics) {
                debugCollisionObjectsRenderer->activateCamera(frameData.at(currentFrame).activeCamera, currentFrame);
            }
            frameData.at(currentFrame).activeCamera = nullptr;
        }
        if (sceneRenderer->getCamera(currentFrame) == nullptr) {
            const auto &camera = rootNode->findFirstChild<Camera>(true);
            if (camera && camera->isProcessed()) {
                sceneRenderer->activateCamera(camera, currentFrame);
                if (applicationConfig.debugPhysics) {
                    debugCollisionObjectsRenderer->activateCamera(camera, currentFrame);
                }
            }
        }
        sceneRenderer->postUpdateScene(currentFrame);
        if (applicationConfig.debugPhysics &&
                (elapsedSeconds >= std::min(0.500f, std::max(0.0f, applicationConfig.debugPhysicsConfig.updateDelay / 1000.0f)))) {
            debugCollisionObjectsRenderer->startDrawing();
            if (applicationConfig.debugPhysicsConfig.drawCoordinateSystem) {
                debugCollisionObjectsRenderer->DrawCoordinateSystem(JPH::RMat44::sIdentity());
            }
            _getPhysicsSystem().DrawBodies(bodyDrawSettings, debugCollisionObjectsRenderer.get(), nullptr);
        }
    }

    void VulkanApplication::setShadowCasting(const bool enable) const {
        sceneRenderer->setShadowCasting(enable);
    }

    float VulkanApplication::getAspectRatio() const { return device->getAspectRatio(); }

    uint64_t VulkanApplication::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string &VulkanApplication::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t VulkanApplication::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }

} // namespace z0
