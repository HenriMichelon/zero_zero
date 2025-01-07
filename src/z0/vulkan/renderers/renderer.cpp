/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.vulkan.Renderer;

import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    Renderer::Renderer(const bool canBeThreaded): threaded{canBeThreaded} {
        const auto& dev = Device::get();
        commandPool = dev.createCommandPool();
        commandBuffers.resize(dev.getFramesInFlight());
        const VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        for (auto i = 0; i < dev.getFramesInFlight(); i++) {
            if (vkAllocateCommandBuffers(dev.getDevice(), &allocInfo, &commandBuffers[i]) != VK_SUCCESS) {
                die("failed to allocate renderer command buffers!");
            }
        }
    }

     Renderer::~Renderer() {
        vkDestroyCommandPool(Device::get().getDevice(), commandPool, nullptr);
    }


}
