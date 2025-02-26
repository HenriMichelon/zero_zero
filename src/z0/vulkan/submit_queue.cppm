/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

export module z0.vulkan.SubmitQueue;

import z0.vulkan.Buffer;

export namespace z0 {

    class SubmitQueue {
    public:

        struct OneTimeCommand {
            const string    location;
            VkCommandPool   commandPool;
            VkCommandBuffer commandBuffer;
        };

        explicit SubmitQueue(const VkQueue& graphicQueue);

        void stop();

        inline auto& getSubmitMutex() { return submitMutex; }

        OneTimeCommand beginOneTimeCommand(const source_location& location = source_location::current());

        void endOneTimeCommand(const OneTimeCommand& oneTimeCommand, bool immediate = false);

        Buffer& createOneTimeBuffer(
            const OneTimeCommand& oneTimeCommand,
            VkDeviceSize       instanceSize,
            uint32_t           instanceCount,
            VkBufferUsageFlags usageFlags,
            VkDeviceSize       minOffsetAlignment = 1);

    private:
        thread::id              mainThreadId;
        // Queue to submit commands to the GPU
        const VkQueue&          graphicQueue;
        // Stop the submit queue thread
        bool                    quit{false};
        // Submission queue
        list<OneTimeCommand>    commands;
        // To synchronize between main thread & submit thread
        mutex                   submitMutex;
        VkFence                 submitFence;

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


        void run();

        void submit(const OneTimeCommand& command);

    public:
        SubmitQueue(const SubmitQueue &) = delete;
        SubmitQueue &operator=(const SubmitQueue &) = delete;
    };


}
