#pragma once

#include "z0/window.h"
#include "z0/application_config.h"

#define VMA_VULKAN_VERSION 1003000
#include "vk_mem_alloc.h"

#include <optional>
#include <vector>

namespace z0 {

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    class Device: public Object {
    public:
        explicit Device(VkInstance vkInstance, const vector<const char*>& requestedLayers,
                        const ApplicationConfig& applicationConfig, const Window& window);
        void cleanup();

        VmaAllocator getAllocator() const { return allocator; }
        VkDevice getDevice() const { return device; }
        VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        VkPhysicalDeviceProperties getDeviceProperties() const { return deviceProperties; }

        VkImageView createImageView(VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspectFlags,
                                    uint32_t mipLevels = 1,
                                    VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D) const;
        void createImage(uint32_t width,
                         uint32_t height,
                         uint32_t mipLevels,
                         VkSampleCountFlagBits numSamples,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image,
                         VkDeviceMemory& imageMemory,
                         VkImageCreateFlags flags = 0,
                         uint32_t layers = 1) const;
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        void transitionImageLayout(VkCommandBuffer commandBuffer,
                                   VkImage image,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout,
                                   VkAccessFlags srcAccessMask,
                                   VkAccessFlags dstAccessMask,
                                   VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask,
                                   VkImageAspectFlags aspectMask,
                                   uint32_t mipLevels = 1) const;
        VkCommandBuffer beginSingleTimeCommands() const;
        void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    private:
        const Window& window;
        VkInstance vkInstance;

        // Physical & logical device management
        VkSurfaceKHR surface;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkCommandPool commandPool;
        VkPhysicalDeviceProperties deviceProperties;
        VkSampleCountFlagBits samples;

        // Vulkan Memory Allocator
        VmaAllocator allocator;

        // Drawing a frame
        uint32_t currentFrame = 0;
        vector<VkCommandBuffer> commandBuffers;
        vector<VkSemaphore> imageAvailableSemaphores;
        vector<VkSemaphore> renderFinishedSemaphores;
        vector<VkFence> inFlightFences;
        VkImageBlit colorImageBlit{};

        // Swap chain management
        VkSwapchainKHR swapChain;
        vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        vector<VkImageView> swapChainImageViews;
        shared_ptr<VkSwapchainKHR> oldSwapChain;
        void createSwapChain();
        void cleanupSwapChain();
        void recreateSwapChain();

        // Check if all the requested Vulkan extensions are supported by a device
        static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice, const vector<const char*>& deviceExtensions);
        // Get the swap chain format, default for sRGB/NON-LINEAR
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
        // Get the swap chain present mode, default to MAILBOX, if not avaible FIFO (V-SYNC)
        static VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes);

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
            VkSurfaceCapabilitiesKHR capabilities;
            vector<VkSurfaceFormatKHR> formats;
            vector<VkPresentModeKHR> presentModes;
        };

        // Rate physical device by properties to find the best suitable GPU
        uint32_t rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, const vector<const char*>& deviceExtensions) const;
        // Get the swap chain capabilities
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice) const;
        // Get the supported queues families for a particular GPU
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;
        // Get the maximum MSAA samples
        VkSampleCountFlagBits getMaxUsableMSAASampleCount() const;
        // Get the swap chain images sizes
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

    public:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
    };

}