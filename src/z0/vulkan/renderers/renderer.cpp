/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
#include "z0/libraries.h"

module z0.vulkan.Renderer;

import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    Renderer::Renderer(const bool canBeThreaded): threaded{canBeThreaded} {
        const auto& dev = Device::get();
        commandPools.push_back(dev.createCommandPool());
        commandBuffers.resize(dev.getFramesInFlight());
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPools.back(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        for (auto i = 0; i < dev.getFramesInFlight(); i++) {
            if (vkAllocateCommandBuffers(dev.getDevice(), &allocInfo, &commandBuffers[i]) != VK_SUCCESS) {
                die("failed to allocate renderer command buffers!");
            }
        }
    }

    vector<VkCommandBuffer> Renderer::getCommandBuffers(const uint32_t currentFrame) const {
        return vector{commandBuffers[currentFrame]};
    }

     Renderer::~Renderer() {
        for (const auto& commandPool : commandPools) {
            vkDestroyCommandPool(Device::get().getDevice(), commandPool, nullptr);
        }
    }


}
