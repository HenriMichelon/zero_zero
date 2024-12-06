/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include <volk.h>

export module z0.vulkan.SubmitQueue;

import z0.vulkan.Buffer;

export namespace z0 {

    class SubmitQueue {
    public:
        struct FrameData {
            VkCommandBuffer   commandBuffer;
            VkSemaphore       imageAvailableSemaphore;
            VkSemaphore       renderFinishedSemaphore;
            VkFence           inFlightFence;
            uint32_t          imageIndex;
        };

        struct OneTimeCommand {
            VkCommandPool   commandPool;
            VkCommandBuffer commandBuffer;
        };

        struct SubmitInfo {
            VkSubmitInfo      submitInfo;
            VkPresentInfoKHR  presentInfo;
            VkFence           fence{VK_NULL_HANDLE};
            OneTimeCommand    command;
        };

        explicit SubmitQueue(const VkQueue& graphicQueue, const VkQueue& presentQueue, uint32_t framesInFlight);

        void submit(FrameData& frameData, VkSwapchainKHR& swapChain);

        void stop();

        inline mutex& getSwapChainMutex() { return swapChainMutex; }

        inline counting_semaphore<5>& getSwapChainSemaphore() { return swapChainSemaphore; }

        OneTimeCommand beginOneTimeCommand();

        void endOneTimeCommand(const OneTimeCommand& oneTimeCommand, bool immediate = false);

        Buffer& createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            VkDeviceSize       instanceSize,
            uint32_t           instanceCount,
            VkBufferUsageFlags usageFlags,
            VkDeviceSize       minOffsetAlignment = 1);

    private:
        // Queue to submit commands to the GPU
        const VkQueue&          graphicQueue;
        // Queue to present swap chain
        const VkQueue&          presentQueue;
        // Stop the submit queue thread
        bool                    quit{false};
        // Submission queue
        list<SubmitInfo>        submitInfos;
        // To prevent present & acquire at the same time
        mutex                   swapChainMutex;
        // To prevent the main thread to acquire images too fast
        counting_semaphore<5>   swapChainSemaphore;

        // The submission thread & locks
        thread                  queueThread;
        mutex                   queueMutex;
        condition_variable      queueCv;

        // Temporary one time command buffers, associated buffers and command pools
        // One command pool per command buffer
        list<OneTimeCommand>    oneTimeCommands;
        mutex                   oneTimeMutex;
        mutex                   oneTimeBuffersMutex;
        map<VkCommandBuffer, list<Buffer>> oneTimeBuffers;

        static constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        void run();

        void submit(const OneTimeCommand& command);

        void submit(const SubmitInfo& submitInfo);

    public:
        SubmitQueue(const SubmitQueue &) = delete;
        SubmitQueue &operator=(const SubmitQueue &) = delete;
    };


}
