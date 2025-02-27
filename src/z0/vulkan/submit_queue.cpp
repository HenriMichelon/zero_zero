/*
* Copyright (c) 2024-2025 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

module z0.vulkan.SubmitQueue;

import z0.Log;
import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    SubmitQueue::SubmitQueue(const VkQueue& graphicQueue) :
        mainThreadId{this_thread::get_id()},
        graphicQueue{graphicQueue},
        queueThread{&SubmitQueue::run, this} {
        constexpr VkFenceCreateInfo fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        const auto& device = Device::get().getDevice();
        vkCreateFence(device, &fenceInfo, nullptr, &submitFence);
    }

    Buffer& SubmitQueue::createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            VkDeviceSize       instanceSize,
            uint32_t           instanceCount,
            VkBufferUsageFlags usageFlags,
            VkDeviceSize       minOffsetAlignment) {
        auto lock = lock_guard(oneTimeBuffersMutex);
        oneTimeBuffers[oneTimeCommand.commandBuffer].emplace_back(instanceSize, instanceCount, usageFlags, minOffsetAlignment);
        return oneTimeBuffers[oneTimeCommand.commandBuffer].back();
    }

    SubmitQueue::OneTimeCommand SubmitQueue::beginOneTimeCommand(const source_location& location) {
        auto lock = lock_guard(oneTimeMutex);
        if (oneTimeCommands.empty()) {
            const auto& device = Device::get();
            const auto commandPool = device.createCommandPool();
            const VkCommandBufferAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
            };
            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &commandBuffer);
            constexpr VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };
            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            stringstream ss;
            ss << location.function_name() << " line " << location.line();
            return {ss.str(), commandPool, commandBuffer};
        }
        const auto command = oneTimeCommands.front();
        oneTimeCommands.pop_front();
        vkResetCommandBuffer(command.commandBuffer, 0);
        constexpr VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(command.commandBuffer, &beginInfo);
        return command;
    }

    void SubmitQueue::endOneTimeCommand(const OneTimeCommand& oneTimeCommand, const bool immediate) {
        vkEndCommandBuffer(oneTimeCommand.commandBuffer);
        if (immediate) {
            auto lock = lock_guard{queueMutex};
            submit(oneTimeCommand);
        } else {
            auto lock = lock_guard{queueMutex};
            commands.push_back(oneTimeCommand);
            queueCv.notify_one();
        }
    }

    void SubmitQueue::run() {
        while (!quit) {
            auto lock = unique_lock{queueMutex};
            queueCv.wait(lock, [this] {
                return quit || !commands.empty();
            });
            if (quit) { break; }
            auto command = commands.front();
            commands.pop_front();
            submit(command);
        }
    }

    void SubmitQueue::submit(const OneTimeCommand& command) {
        const auto vkSubmitInfo = VkSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &command.commandBuffer,
        };
        const auto&device = Device::get().getDevice();
        // log("queue start submit");
        {
            if (vkWaitForFences(device, 1, &submitFence, VK_TRUE, 0) == VK_TIMEOUT) {
                WARNING("SubmitQueue : skip submition ", command.location);
                return;
            }

            const auto lock_queue = lock_guard(getSubmitMutex());
            vkResetFences(device, 1, &submitFence);
            if (vkQueueSubmit(graphicQueue, 1, &vkSubmitInfo, submitFence) != VK_SUCCESS) {
                die("failed to submit draw command buffer!");
            }
            // wait the commands to be completed before destroying command buffer
            vkQueueWaitIdle(graphicQueue);
            const auto lock_commands = lock_guard(oneTimeMutex);
            oneTimeCommands.push_back(command);
            {
                auto lockBuffer = lock_guard(oneTimeBuffersMutex);
                oneTimeBuffers.erase(command.commandBuffer);
            }
        }
        // log("queue end submit");
    }

    void SubmitQueue::stop() {
        quit = true;
        queueCv.notify_one();
        queueThread.join();
        const auto &device = Device::get().getDevice();
        for (const auto& command : oneTimeCommands) {
            vkDestroyCommandPool(device, command.commandPool, nullptr);
        }
        vkDestroyFence(device, submitFence, nullptr);
    }

}