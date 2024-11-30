/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <volk.h>

export module z0.SubmitQueue;

export namespace z0 {

    class SubmitQueue {
    public:
        struct FrameData {
            VkCommandBuffer commandBuffer;
            VkSemaphore     imageAvailableSemaphore;
            VkSemaphore     renderFinishedSemaphore;
            VkFence         inFlightFence;
            uint32_t        imageIndex;
        };

        struct SubmitInfo {
            VkSubmitInfo         submitInfo;
            VkPresentInfoKHR     presentInfo;
            VkFence              fence{VK_NULL_HANDLE};
        };

        explicit SubmitQueue(const VkQueue& graphicQueue, const VkQueue& presentQueue);

        void submit(VkCommandBuffer commandBuffer);

        void submit(const FrameData& frameData, VkSwapchainKHR& swapChain);

        void stop();

        inline mutex& getSwapChainMutex() { return swapChainMutex; }

    private:
        const VkQueue&     graphicQueue;
        const VkQueue&     presentQueue;
        bool               quit{false};
        list<SubmitInfo>   submitInfos;
        thread             queueThread;
        mutex              queueMutex;
        condition_variable queueCv;
        mutex              swapChainMutex;

        static constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        void run();

    public:
        SubmitQueue(const SubmitQueue &) = delete;
        SubmitQueue &operator=(const SubmitQueue &) = delete;
    };


}
