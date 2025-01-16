/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#include <dxgi1_4.h>
#endif
#include "z0/libraries.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "vk_mem_alloc.h"

export module z0.vulkan.Device;

import z0.Constants;
import z0.ApplicationConfig;
import z0.Window;

import z0.vulkan.Buffer;
import z0.vulkan.Renderer;
import z0.vulkan.SubmitQueue;

export namespace z0 {

    /*
     * Vulkan [VkDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html) helper
     */
    class Device {
    public:
        static Device &get() {
            return *_instance;
        }

        Device(Device &device) = delete;
        Device(Device &&device) = delete;

        explicit Device(VkInstance              instance,
                    const vector<const char *> &requestedLayers,
                    const ApplicationConfig &   applicationConfig,
                    const Window &              theWindow);

        void cleanup();

        [[nodiscard]] inline VmaAllocator getAllocator() const { return allocator; }

        [[nodiscard]] inline VkDevice getDevice() const { return device; }

        [[nodiscard]] inline VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

        [[nodiscard]] inline VkPhysicalDeviceFeatures getDeviceFeatures() const { return deviceFeatures; }

        [[nodiscard]] inline VkPhysicalDeviceProperties getDeviceProperties() const { return deviceProperties.properties; }

        [[nodiscard]] inline VkSampleCountFlagBits getSamples() const { return samples; }

        [[nodiscard]] inline const VkExtent2D &getSwapChainExtent() const { return swapChainExtent; }

        [[nodiscard]] inline VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }

        [[nodiscard]] bool isFormatSupported(VkFormat format) const;

        [[nodiscard]] inline float getAspectRatio() const {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        [[nodiscard]] inline uint64_t getDedicatedVideoMemory() const { return dedicatedVideoMemory; }

        [[nodiscard]] inline const string &getAdapterDescription() const { return adapterDescription; }

        [[nodiscard]] uint64_t getVideoMemoryUsage() const;

        [[nodiscard]] inline uint32_t getFramesInFlight() const { return framesInFlight; }

        void drawFrame(uint32_t currentFrame);

        void wait() const;

        void stop();

        void registerRenderer(const shared_ptr<Renderer> &renderer);

        void unRegisterRenderer(const shared_ptr<Renderer> &renderer, bool immediate);

        [[nodiscard]] VkImageView createImageView(VkImage            image,
                                                  VkFormat           format,
                                                  VkImageAspectFlags aspectFlags,
                                                  uint32_t           mipLevels = 1,
                                                  VkImageViewType    type      = VK_IMAGE_VIEW_TYPE_2D,
                                                  uint32_t           baseArrayLayer = 0,
                                                  uint32_t           layers = 1,
                                                  uint32_t           baseMipLevel = 0) const;

        void createImage(uint32_t              width,
                         uint32_t              height,
                         uint32_t              mipLevels,
                         VkSampleCountFlagBits numSamples,
                         VkFormat              format,
                         VkImageTiling         tiling,
                         VkImageUsageFlags     usage,
                         VkMemoryPropertyFlags properties,
                         VkImage &             image,
                         VkDeviceMemory &      imageMemory,
                         VkImageCreateFlags    flags  = 0,
                         uint32_t              layers = 1) const;

        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

        static void transitionImageLayout(VkCommandBuffer      commandBuffer,
                                          VkImage              image,
                                          VkImageLayout        oldLayout,
                                          VkImageLayout        newLayout,
                                          VkAccessFlags        srcAccessMask,
                                          VkAccessFlags        dstAccessMask,
                                          VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask,
                                          VkImageAspectFlags   aspectMask,
                                          uint32_t             mipLevels = 1);

        // Returns true if a given format support LINEAR filtering
        [[nodiscard]] VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling) const;

        // Find a suitable IMAGE_TILING format (for the Depth buffering image)
        [[nodiscard]] VkFormat findImageSupportedFormat(const vector<VkFormat> &candidates,
                                                              VkImageTiling           tiling,
                                                              VkFormatFeatureFlags    features) const;

        [[nodiscard]] VkCommandPool createCommandPool() const;

        [[nodiscard]] SubmitQueue::OneTimeCommand beginOneTimeCommandBuffer() const;

        void endOneTimeCommandBuffer(const SubmitQueue::OneTimeCommand& commandBuffer, bool immediate = false) const;

        inline Buffer& createOneTimeBuffer(
           const SubmitQueue::OneTimeCommand& oneTimeCommand,
           VkDeviceSize       instanceSize,
           uint32_t           instanceCount,
           VkBufferUsageFlags usageFlags,
           VkDeviceSize       minOffsetAlignment = 1) const {
            return submitQueue->createOneTimeBuffer(oneTimeCommand, instanceSize, instanceCount, usageFlags, minOffsetAlignment);
        }

    private:
        static Device *             _instance;
        const Window &              window;
        VkInstance                  vkInstance;
        VmaAllocator                allocator;

        // Physical & logical device management
        VkSurfaceKHR                surface;
        VkDevice                    device;
        VkPhysicalDevice            physicalDevice;
        VkQueue                     graphicsQueue;
        VkQueue                     presentQueue;
        VkPhysicalDeviceFeatures    deviceFeatures {};
        VkPhysicalDeviceProperties2 deviceProperties{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        VkPhysicalDeviceIDProperties physDeviceIDProps{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES
        };
        VkSampleCountFlagBits        samples;

        unique_ptr<SubmitQueue>      submitQueue;
        list<shared_ptr<Renderer>>   renderers;
        list<shared_ptr<Renderer>>   renderersToRemove;
        mutex                        renderersToRemoveMutex;

        // Total video memory given by the OS (not Vulkan)
        uint64_t                     dedicatedVideoMemory;
        // GPU description given by the OS (not Vulkan)
        string                       adapterDescription;
#ifdef _WIN32
        IDXGIAdapter3 *              dxgiAdapter{nullptr};
#endif

        // Drawing a frame
        struct FrameData {
            VkSemaphore       imageAvailableSemaphore;
            VkSemaphore       renderFinishedSemaphore;
            VkFence           inFlightFence;
            uint32_t          imageIndex;
            unique_ptr<thread>renderThread;
        };
        vector<FrameData> framesData;
        const uint32_t    framesInFlight;
        VkImageBlit       colorImageBlit{};
        static constexpr VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        // Swap chain management
        VkSwapchainKHR         swapChain;
        mutex                  swapChainMutex;
        vector<VkImage>        swapChainImages;
        VkFormat               swapChainImageFormat;
        VkExtent2D             swapChainExtent;
        vector<VkImageView>    swapChainImageViews;

        void createSwapChain();

        void cleanupSwapChain() const;

        void recreateSwapChain();

        void renderFrame(uint32_t currentFrame);

        // Check if all the requested Vulkan extensions are supported by a device
        [[nodiscard]] static bool checkDeviceExtensionSupport(VkPhysicalDevice            vkPhysicalDevice,
                                                              const vector<const char *> &deviceExtensions);

        // Get the swap chain format, default for sRGB/NON-LINEAR
        [[nodiscard]] static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                const vector<VkSurfaceFormatKHR> &availableFormats);

        // Get the swap chain present mode, default to MAILBOX, if not available FIFO (V-SYNC)
        [[nodiscard]] static VkPresentModeKHR chooseSwapPresentMode(
                const vector<VkPresentModeKHR> &availablePresentModes);

        // For Device::findQueueFamilies()
        struct QueueFamilyIndices {
            optional<uint32_t> graphicsFamily;
            optional<uint32_t> presentFamily;

            bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        // For Device::querySwapChainSupport()
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR   capabilities;
            vector<VkSurfaceFormatKHR> formats;
            vector<VkPresentModeKHR>   presentModes;
        };

        // Set the initial state for the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer) const;

        // Rate physical device by properties to find the best suitable GPU
        [[nodiscard]] uint32_t rateDeviceSuitability(VkPhysicalDevice            vkPhysicalDevice,
                                                     const vector<const char *> &deviceExtensions) const;

        // Get the swap chain capabilities
        [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice) const;

        // Get the supported queues families for a particular GPU
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;

        uint32_t getFirstGraphicQueueCount(VkPhysicalDevice physicalDevice) const;

        // Get the maximum MSAA samples
        [[nodiscard]] VkSampleCountFlagBits getMaxUsableMSAASampleCount() const;

        // Get the swap chain images sizes
        [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        // Get the GPU details from the OS
        void getAdapterDescFromOS();
    };
}
