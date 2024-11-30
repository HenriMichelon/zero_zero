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

import z0.Buffer;

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

        struct OneTimeCommand {
            VkCommandPool   commandPool;
            VkCommandBuffer commandBuffer;
        };

        struct SubmitInfo {
            VkSubmitInfo     submitInfo;
            VkPresentInfoKHR presentInfo;
            VkFence          fence{VK_NULL_HANDLE};
            OneTimeCommand   command;
        };

        explicit SubmitQueue(const VkQueue& graphicQueue, const VkQueue& presentQueue);

        void submit(const FrameData& frameData, VkSwapchainKHR& swapChain);

        void stop();

        inline mutex& getSwapChainMutex() { return swapChainMutex; }

        OneTimeCommand beginOneTimeCommand();

        OneTimeCommand beginOneTimeCommand(const Buffer& buffer);

        void endOneTimeCommand(const OneTimeCommand& oneTimeCommand);

        Buffer& createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            VkDeviceSize       instanceSize,
            uint32_t           instanceCount,
            VkBufferUsageFlags usageFlags,
            VkDeviceSize       minOffsetAlignment = 1);

    private:
        const VkQueue&     graphicQueue;
        const VkQueue&     presentQueue;
        bool               quit{false};
        list<SubmitInfo>   submitInfos;
        thread             queueThread;
        mutex              queueMutex;
        condition_variable queueCv;
        mutex              swapChainMutex;

        list<OneTimeCommand> oneTimeCommands;
        mutex                oneTimeMutex;
        map<VkCommandBuffer, list<Buffer>> oneTimeBuffers;
        mutex oneTimeBuffersMutex;

        static constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        void run();

        void submit(const OneTimeCommand& command);

    public:
        SubmitQueue(const SubmitQueue &) = delete;
        SubmitQueue &operator=(const SubmitQueue &) = delete;
    };


}
