/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include "z0/libraries.h"

module z0.vulkan.Application;

import z0.ApplicationConfig;
import z0.Tools;

import z0.nodes.Camera;
import z0.nodes.Node;

import z0.resources.Material;

import z0.ui.Manager;

import z0.vulkan.DebugRenderer;
import z0.vulkan.Device;
import z0.vulkan.Instance;
import z0.vulkan.SceneRenderer;
import z0.vulkan.VectorRenderer;
import z0.vulkan.TonemappingPostprocessingRenderer;
import z0.vulkan.Image;

namespace z0 {

    VulkanApplication::VulkanApplication(const ApplicationConfig &appConfig, const shared_ptr<Node> &node) :
        Application{appConfig, node} {
        assert(window != nullptr);
        // creates the Application singleton
        instance = make_unique<Instance>();
        // creates the Vulkan device
        device = instance->createDevice(applicationConfig, *window);
        // initialize Jolt debug parameters from the application config
        if (appConfig.debug) {
            const auto& conf = appConfig.debugConfig;
            bodyDrawSettings = JPH::BodyManager::DrawSettings{
                .mDrawGetSupportFunction = conf.drawGetSupportingFace,
                .mDrawShape = conf.drawShape,
                .mDrawShapeWireframe = conf.drawShapeWireframe,
                .mDrawShapeColor = static_cast<JPH::BodyManager::EShapeColor>(conf.drawShapeColor),
                .mDrawBoundingBox = conf.drawBoundingBox,
                .mDrawCenterOfMassTransform = conf.drawCenterOfMassTransform,
                .mDrawWorldTransform = conf.drawWorldTransform,
                .mDrawVelocity = conf.drawVelocity,
                .mDrawMassAndInertia = conf.drawMassAndInertia,
                .mDrawSleepStats = conf.drawSleepStats,
            };
        }
        // Call the implementation independant initialization code
        init();
    }

    VulkanApplication::~VulkanApplication() {
        // destroy the variou renderers
        sceneRenderer.reset();
        if (vectorRenderer) { vectorRenderer.reset(); }
        if (tonemappingRenderer) { tonemappingRenderer.reset(); }
        if (debugRenderer) { debugRenderer.reset(); }
        // shutdown the Vulkan device & instance
        device->cleanup();
        device.reset();
        instance.reset();
    }

    void VulkanApplication::initRenderingSystem() {
        // create the main renderer
        sceneRenderer = make_shared<SceneRenderer>(
            *device,
            applicationConfig.clearColor,
            applicationConfig.useDepthPrepass);
        // create the HDR tone mapping renderer
        tonemappingRenderer = make_shared<TonemappingPostprocessingRenderer>(
             *device,
             sceneRenderer->getColorAttachments(),
             sceneRenderer->getDepthAttachments());
        // create the vector renderer used by the UI components
        vectorRenderer = make_shared<VectorRenderer>(
            *device,
            tonemappingRenderer->getColorAttachments());

        // Register the renderers in order, the first registered will be the last executed
        device->registerRenderer(vectorRenderer);
        device->registerRenderer(tonemappingRenderer);
        if (applicationConfig.debug) {
            debugRenderer = make_shared<DebugRenderer>(
                *device,
                sceneRenderer->getColorAttachments(),
                sceneRenderer->getDepthAttachments(),
                applicationConfig.debugConfig.drawWithDepthTest);
            device->registerRenderer(debugRenderer);
        }
        device->registerRenderer(sceneRenderer);

        // Create the UI windows manager
        windowManager = make_unique<ui::Manager>(vectorRenderer,
                                              applicationConfig.defaultFontName,
                                              applicationConfig.defaultFontSize);
    }

    void VulkanApplication::stopRenderingSystem() {
        // Stop submitting to the command queues
        device->stop();
    }

    void VulkanApplication::waitForRenderingSystem() {
        // Wait for all Vulkan queues to finish the current work
        device->wait();
    }

    void VulkanApplication::processDeferredUpdates(const uint32_t currentFrame) {
        // Update renderer resources
        sceneRenderer->preUpdateScene(currentFrame);
        // Register UI drawing commands
        windowManager->drawFrame();
        {
            auto lock = lock_guard(frameDataMutex);
            auto &data = frameData[currentFrame];
            // Remove from the renderer the nodes previously removed from the scene tree
            // Immediate removes
            if (!data.removedNodes.empty()) {
                for (const auto &node : data.removedNodes) {
                    sceneRenderer->removeNode(node, currentFrame);
                }
                data.removedNodes.clear();
            }
            // Batched removes
            if (!data.removedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.removedNodesAsync.begin(); it != data.removedNodesAsync.end();) {
                    sceneRenderer->removeNode(*it, currentFrame);
                    it = data.removedNodesAsync.erase(it);
                    count += 1;
                    if (count > MAX_ASYNC_NODE_OP_PER_FRAME) { break; }
                }
            }
            // Add to the renderer the nodes previously added to the scene tree
            // Immediate additions
            if (!data.addedNodes.empty()) {
                for (const auto &node : data.addedNodes) {
                    sceneRenderer->addNode(node, currentFrame);
                }
                data.addedNodes.clear();
            }
            // Batched additions
            if (!data.addedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.addedNodesAsync.begin(); it != data.addedNodesAsync.end();) {
                    sceneRenderer->addNode(*it, currentFrame);
                    it = data.addedNodesAsync.erase(it);
                    count += 1;
                    if (count > MAX_ASYNC_NODE_OP_PER_FRAME) { break; }
                }
            }
            // Change current camera if needed
            if (data.cameraChanged) {
                sceneRenderer->activateCamera(data.activeCamera, currentFrame);
                if (applicationConfig.debug) {
                    debugRenderer->activateCamera(data.activeCamera, currentFrame);
                }
                data.activeCamera.reset();
                data.cameraChanged = false;
            }
            // Search for a camera in the scene tree if not current camera
            if (sceneRenderer->getCamera(currentFrame) == nullptr) {
                const auto &camera = rootNode->findFirstChild<Camera>(true);
                if (camera && camera->isProcessed()) {
                    sceneRenderer->activateCamera(camera, currentFrame);
                    if (applicationConfig.debug) {
                        debugRenderer->activateCamera(camera, currentFrame);
                    }
                }
            }
        }
        // Update renderer resources
        sceneRenderer->postUpdateScene(currentFrame);
        // Register commands for the in-game debug layer
        if (applicationConfig.debug && displayDebug &&
                (elapsedSeconds >= std::min(0.500f, std::max(0.0f, applicationConfig.debugConfig.updateDelay / 1000.0f)))) {
            debugRenderer->startDrawing();
            if (applicationConfig.debugConfig.drawCoordinateSystem) {
                debugRenderer->DrawCoordinateSystem(JPH::RMat44::sTranslation(
                        JPH::Vec3(
                            applicationConfig.debugConfig.drawCoordinateSystemPosition.x,
                            applicationConfig.debugConfig.drawCoordinateSystemPosition.y,
                            applicationConfig.debugConfig.drawCoordinateSystemPosition.z)) *
                    JPH::Mat44::sScale(applicationConfig.debugConfig.drawCoordinateSystemScale));
            }
            if (applicationConfig.debugConfig.drawRayCast) {
                debugRenderer->drawRayCasts(
                    rootNode,
                    applicationConfig.debugConfig.drawRayCastColor,
                    applicationConfig.debugConfig.drawRayCastCollidingColor);
            }
            _getPhysicsSystem().DrawBodies(bodyDrawSettings, debugRenderer.get(), nullptr);
        }
    }

    void VulkanApplication::setShadowCasting(const bool enable) const {
        // Globally enable or disable the shadows
        sceneRenderer->setShadowCasting(enable);
    }

    float VulkanApplication::getAspectRatio() const { return device->getAspectRatio(); }

    uint64_t VulkanApplication::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string &VulkanApplication::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t VulkanApplication::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }

} // namespace z0
