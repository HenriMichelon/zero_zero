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
        instance = make_unique<Instance>();
        device = instance->createDevice(applicationConfig, *window);
        // KTXVulkanImage::initialize(
        //     device->getPhysicalDevice(), device->getDevice(),
        //     device->getGraphicQueue(), device->getCommandPool());
        const auto& conf = appConfig.debugConfig;
        if (appConfig.debug) {
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
        init();
    }

    VulkanApplication::~VulkanApplication() {
        sceneRenderer.reset();
        if (vectorRenderer) { vectorRenderer.reset(); }
        if (tonemappingRenderer) { tonemappingRenderer.reset(); }
        if (debugRenderer) { debugRenderer.reset(); }
        // KTXVulkanImage::cleanup();
        device->cleanup();
        device.reset();
        instance.reset();
    }

    void VulkanApplication::initRenderingSystem() {
        sceneRenderer = make_shared<SceneRenderer>(
            *device,
            applicationConfig.clearColor,
            applicationConfig.useDepthPrepass);
        tonemappingRenderer = make_shared<TonemappingPostprocessingRenderer>(
            *device,
            sceneRenderer->getColorAttachments(),
            sceneRenderer->getDepthAttachments());
        vectorRenderer = make_shared<VectorRenderer>(
            *device,
            tonemappingRenderer->getColorAttachments());

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

        windowManager = make_unique<ui::Manager>(vectorRenderer,
                                              applicationConfig.defaultFontName,
                                              applicationConfig.defaultFontSize);
    }

    void VulkanApplication::stopRenderingSystem() {
        device->stop();
    }

    void VulkanApplication::waitForRenderingSystem() {
        device->wait();
    }

    void VulkanApplication::processDeferredUpdates(const uint32_t currentFrame) {
        sceneRenderer->preUpdateScene(currentFrame);
        windowManager->drawFrame();
        {
            auto lock = lock_guard(frameDataMutex);
            auto &data = frameData[currentFrame];
            if (!data.removedNodes.empty()) {
                for (const auto &node : data.removedNodes) {
                    sceneRenderer->removeNode(node, currentFrame);
                }
                data.removedNodes.clear();
            }
            if (!data.removedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.removedNodesAsync.begin(); it != data.removedNodesAsync.end();) {
                    sceneRenderer->removeNode(*it, currentFrame);
                    it = data.removedNodesAsync.erase(it);
                    count += 1;
                    if (count > MAX_ASYNC_NODE_OP_PER_FRAME) { break; }
                }
            }
            if (!data.addedNodes.empty()) {
                for (const auto &node : data.addedNodes) {
                    sceneRenderer->addNode(node, currentFrame);
                }
                data.addedNodes.clear();
            }
            if (!data.addedNodesAsync.empty()) {
                auto count = 0;
                for (auto it = data.addedNodesAsync.begin(); it != data.addedNodesAsync.end();) {
                    sceneRenderer->addNode(*it, currentFrame);
                    it = data.addedNodesAsync.erase(it);
                    count += 1;
                    if (count > MAX_ASYNC_NODE_OP_PER_FRAME) { break; }
                }
            }
            if (data.activeCamera != nullptr) {
                sceneRenderer->activateCamera(data.activeCamera, currentFrame);
                if (applicationConfig.debug) {
                    debugRenderer->activateCamera(data.activeCamera, currentFrame);
                }
                data.activeCamera = nullptr;
            }
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
        sceneRenderer->postUpdateScene(currentFrame);
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
        sceneRenderer->setShadowCasting(enable);
    }

    float VulkanApplication::getAspectRatio() const { return device->getAspectRatio(); }

    uint64_t VulkanApplication::getDedicatedVideoMemory() const { return device->getDedicatedVideoMemory(); }

    const string &VulkanApplication::getAdapterDescription() const { return device->getAdapterDescription(); }

    uint64_t VulkanApplication::getVideoMemoryUsage() const { return device->getVideoMemoryUsage(); }

} // namespace z0
