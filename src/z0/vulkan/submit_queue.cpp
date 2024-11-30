/*
* Copyright (c) 2024 Henri Michelon
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

module z0.SubmitQueue;

import z0.Device;
import z0.Tools;

namespace z0 {

    SubmitQueue::SubmitQueue(const VkQueue& graphicQueue, const VkQueue& presentQueue) :
        graphicQueue{graphicQueue},
        presentQueue{presentQueue},
        queueThread{&SubmitQueue::run, this} {
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
            if (vkQueueSubmit(graphicQueue, 1, &submitInfo.submitInfo, submitInfo.fence) != VK_SUCCESS) {
                die("failed to submit draw command buffer!");
            }
            if (submitInfo.fence != VK_NULL_HANDLE) {
                auto swapChainLock = unique_lock{swapChainMutex};
                if (vkQueuePresentKHR(presentQueue, &submitInfo.presentInfo) != VK_SUCCESS) {
                    die("failed to present swap chain image!");
                }
            } else {
                log("SubmitQueue process additional");
            }
        }
    }

    void SubmitQueue::submit(const VkCommandBuffer commandBuffer) {
        {
            auto lock = lock_guard{queueMutex};
            submitInfos.push_back({VkSubmitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
            }});
        }
        queueCv.notify_one();
    }

    void SubmitQueue::submit(const FrameData& data, VkSwapchainKHR& swapChain) {
        {
            auto lock = lock_guard{queueMutex};
            submitInfos.push_back({
                {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &data.imageAvailableSemaphore,
                    .pWaitDstStageMask = waitStages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &data.commandBuffer,
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &data.renderFinishedSemaphore
                },
    {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &data.renderFinishedSemaphore,
                    .swapchainCount = 1,
                    .pSwapchains = &swapChain,
                    .pImageIndices = &data.imageIndex,
                    .pResults = nullptr // Optional
                },
                data.inFlightFence
            });
        }
        queueCv.notify_one();
    }

    void SubmitQueue::stop() {
        quit = true;
        queueCv.notify_one();
        queueThread.join();
    }

}