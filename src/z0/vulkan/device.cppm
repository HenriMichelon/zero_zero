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

        [[nodiscard]] inline auto getAllocator() const { return allocator; }

        [[nodiscard]] inline auto getDevice() const { return device; }

        [[nodiscard]] inline auto getPhysicalDevice() const { return physicalDevice; }

        [[nodiscard]] inline auto getDeviceFeatures() const { return deviceFeatures; }

        [[nodiscard]] inline auto getDeviceProperties() const { return deviceProperties.properties; }

        [[nodiscard]] inline auto getSamples() const { return samples; }

        [[nodiscard]] inline const auto &getSwapChainExtent() const { return swapChainExtent; }

        [[nodiscard]] inline auto getSwapChainImageFormat() const { return swapChainImageFormat; }

        [[nodiscard]] bool isFormatSupported(VkFormat format) const;

        [[nodiscard]] inline auto getAspectRatio() const { return swapChainRatio; }

        [[nodiscard]] inline auto getDedicatedVideoMemory() const { return dedicatedVideoMemory; }

        [[nodiscard]] inline const auto &getAdapterDescription() const { return adapterDescription; }

        [[nodiscard]] uint64_t getVideoMemoryUsage() const;

        [[nodiscard]] inline auto getFramesInFlight() const { return framesInFlight; }

        [[nodiscard]] inline auto getGraphicsQueue() const { return graphicsQueue; }

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

        [[nodiscard]] VkCommandPool createCommandPool(bool isForTransfert = false) const;

        [[nodiscard]] inline auto beginOneTimeCommandBuffer(const source_location& location = source_location::current()) const  {
            return submitQueue->beginOneTimeCommand(location);
        }

        void endOneTimeCommandBuffer(const SubmitQueue::OneTimeCommand& commandBuffer, bool immediate = false) const;

        inline auto& createOneTimeBuffer(
           const SubmitQueue::OneTimeCommand& oneTimeCommand,
           const VkDeviceSize       instanceSize,
           const uint32_t           instanceCount,
           const VkBufferUsageFlags usageFlags,
           const VkDeviceSize       minOffsetAlignment = 1) const {
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
        VkPhysicalDeviceFeatures    deviceFeatures {};
        VkPhysicalDeviceProperties2 deviceProperties{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        VkPhysicalDeviceIDProperties physDeviceIDProps{
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES
        };
        VkSampleCountFlagBits        samples;

        // Command queues & families
        VkQueue                     presentQueue;
        uint32_t                    presentQueueFamilyIndex;
        VkQueue                     graphicsQueue;
        uint32_t                    graphicsQueueFamilyIndex;
        VkQueue                     transferQueue;
        uint32_t                    transfertQueueFamilyIndex;

        // Threaded queue to submit one time commands
        unique_ptr<SubmitQueue>      submitQueue;
        // List of current renderers
        list<shared_ptr<Renderer>>   renderers;
        // List of renderers to remove at the start of the next frame
        list<shared_ptr<Renderer>>   renderersToRemove;
        mutex                        renderersToRemoveMutex;

        // Total video memory given by the OS (not Vulkan)
        uint64_t                     dedicatedVideoMemory;
        // GPU description given by the OS (not Vulkan)
        string                       adapterDescription;
#ifdef _WIN32
        IDXGIAdapter3 *              dxgiAdapter{nullptr};
#endif

        // per-frame data
        struct FrameData {
            VkSemaphore             imageAvailableSemaphore;
            VkSemaphoreSubmitInfo   imageAvailableSemaphoreSubmitInfo;
            VkSemaphore             renderFinishedSemaphore;
            VkSemaphoreSubmitInfo   renderFinishedSemaphoreSubmitInfo;
            VkFence                 inFlightFence;
            uint32_t                imageIndex;
            unique_ptr<thread>      renderThread;
        };
        vector<FrameData> framesData;
        // number of frames in flight
        const uint32_t    framesInFlight;
        // swap chain image blit parameters
        VkImageBlit       colorImageBlit{};

        // Swap chain management
        VkSwapchainKHR         swapChain;
        mutex                  swapChainMutex;
        vector<VkImage>        swapChainImages;
        VkFormat               swapChainImageFormat;
        VkExtent2D             swapChainExtent;
        float                  swapChainRatio;
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
            optional<uint32_t> transfertFamily;

            bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value() & transfertFamily.has_value();
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

        // Find a dedicated tranfer queue
        [[nodiscard]] uint32_t findTransferQueueFamily() const;

        uint32_t getFirstGraphicQueueCount(VkPhysicalDevice physicalDevice) const;

        // Get the maximum MSAA samples
        [[nodiscard]] VkSampleCountFlagBits getMaxUsableMSAASampleCount() const;

        // Get the swap chain images sizes
        [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

        // Get the GPU details from the OS
        void getAdapterDescFromOS();
    };
}
