/*
* Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.vulkan.SubmitQueue;

import z0.Tools;

import z0.vulkan.Device;

namespace z0 {

    SubmitQueue::SubmitQueue(const VkQueue& graphicQueue, const VkQueue& presentQueue, const uint32_t framesInFlight) :
        graphicQueue{graphicQueue},
        presentQueue{presentQueue},
        queueThread{&SubmitQueue::run, this},
        swapChainSemaphore{framesInFlight-1} {
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

    SubmitQueue::OneTimeCommand SubmitQueue::beginOneTimeCommand() {
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
            return {commandPool, commandBuffer};
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

    void SubmitQueue::endOneTimeCommand(const SubmitQueue::OneTimeCommand& oneTimeCommand, const bool immediate) {
        vkEndCommandBuffer(oneTimeCommand.commandBuffer);
        if (immediate) {
            auto lock = lock_guard{queueMutex};
            submit({.command = oneTimeCommand});
        } else {
            submit(oneTimeCommand);
        }
    }

    void SubmitQueue::run() {
        while (!quit) {
            auto lock = unique_lock{queueMutex};
            queueCv.wait(lock, [this] {
                return quit || !submitInfos.empty();
            });
            if (quit) { break; }
            auto submitInfo = submitInfos.front();
            submitInfos.pop_front();
            if (submitInfo.fence != VK_NULL_HANDLE) {
                if (vkQueueSubmit(graphicQueue, 1, &submitInfo.submitInfo, submitInfo.fence) != VK_SUCCESS) {
                    die("failed to submit draw command buffer!");
                }
                {
                    const auto swapChainLock = lock_guard{swapChainMutex};
                    if (vkQueuePresentKHR(presentQueue, &submitInfo.presentInfo) != VK_SUCCESS) {
                        die("failed to present swap chain image!");
                    }
                    swapChainSemaphore.release();
                }
            } else {
                submit(submitInfo);
            }
        }
    }

    void SubmitQueue::submit(const SubmitInfo& submitInfo) {
        const auto vkSubmitInfo = VkSubmitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &submitInfo.command.commandBuffer,
        };
        if (vkQueueSubmit(graphicQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            die("failed to submit draw command buffer!");
        }
        vkQueueWaitIdle(graphicQueue);
        {
            const auto lock = lock_guard(oneTimeMutex);
            oneTimeCommands.push_back(submitInfo.command);
            {
                auto lockBuffer = lock_guard(oneTimeBuffersMutex);
                oneTimeBuffers.erase(submitInfo.command.commandBuffer);
            }
        }
    }

    void SubmitQueue::submit(const OneTimeCommand& oneTimeCommand) {
        if (quit) { return; }
        {
            auto lock = lock_guard{queueMutex};
            submitInfos.push_back({.command = oneTimeCommand});
        }
        queueCv.notify_one();
    }

    void SubmitQueue::submit(FrameData& data, VkSwapchainKHR& swapChain) {
        if (quit) { return; }
        {
            auto lock = lock_guard{queueMutex};
            submitInfos.push_back({
                .submitInfo = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &data.imageAvailableSemaphore,
                    .pWaitDstStageMask = waitStages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &data.commandBuffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &data.renderFinishedSemaphore
                },
                .presentInfo = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &data.renderFinishedSemaphore,
                    .swapchainCount = 1,
                    .pSwapchains = &swapChain,
                    .pImageIndices = &data.imageIndex,
                    .pResults = nullptr // Optional
                },
                .fence = data.inFlightFence,
            });
        }
        queueCv.notify_one();
    }

    void SubmitQueue::stop() {
        quit = true;
        queueCv.notify_one();
        queueThread.join();
        const auto &device = Device::get().getDevice();
        for (const auto& command : oneTimeCommands) {
            vkDestroyCommandPool(device, command.commandPool, nullptr);
        }
    }

}