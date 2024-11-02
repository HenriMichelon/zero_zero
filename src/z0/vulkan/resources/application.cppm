/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"

export module z0:VulkanApplication;

import :Application;
import :ApplicationConfig;
import :Node;

import :Instance;
import :Device;
import :SceneRenderer;
import :VectorRenderer;
import :TonemappingPostprocessingRenderer;

export namespace z0 {

    class VulkanApplication : public Application {
    public:
        explicit VulkanApplication(const ApplicationConfig &applicationConfig, const shared_ptr<Node> &rootNode);

        ~VulkanApplication() override;

        float getAspectRatio() const override;

        uint64_t getDedicatedVideoMemory() const override;

        const string &getAdapterDescription() const override;

        uint64_t getVideoMemoryUsage() const override;

        void initRenderingSystem() override;

        void processDeferredUpdates(uint32_t currentFrame) override;

        inline void renderFrame(const uint32_t currentFrame) override {  device->drawFrame(currentFrame); }

        inline void waitForRenderingSystem() override { device->wait(); }

        void setShadowCasting(const bool enable) const override;

    private:
        // The Vulkan device helper object
        unique_ptr<Device> device;
        // The Vulkan global instance
        unique_ptr<Instance> instance;
        // The Main renderer
        shared_ptr<SceneRenderer> sceneRenderer;
        // The 2D vector renderer used for the UI
        shared_ptr<VectorRenderer> vectorRenderer;
        // HDR & Gamma correction renderer
        shared_ptr<TonemappingPostprocessingRenderer> tonemappingRenderer;
    };
}
