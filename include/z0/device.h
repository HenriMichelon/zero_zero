#pragma once

namespace z0 {

    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    /**
     * Vulkan [VkDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDevice.html) helper
     */
    class Device {
    public:
        explicit Device(VkInstance vkInstance, const vector<const char*>& requestedLayers,
                        const ApplicationConfig& applicationConfig, const Window& window);
        void cleanup();

        [[nodiscard]] VmaAllocator getAllocator() const { return allocator; }
        [[nodiscard]] VkDevice getDevice() const { return device; }
        [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
        [[nodiscard]] VkPhysicalDeviceProperties getDeviceProperties() const { return deviceProperties.properties; }
        [[nodiscard]] VkSampleCountFlagBits getSamples() const { return samples; }
        [[nodiscard]] const VkExtent2D& getSwapChainExtent() const { return swapChainExtent;}
        [[nodiscard]] VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
        [[nodiscard]] float getAspectRatio() const {return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);}
        [[nodiscard]] uint64_t getDedicatedVideoMemory() const { return dedicatedVideoMemory; }
        [[nodiscard]] const string& getAdapterDescription() const { return adapterDescription; }
        [[nodiscard]] uint64_t getVideoMemoryUsage() const;

        void drawFrame();
        void wait() const;
        void registerRenderer(const shared_ptr<Renderer>& renderer);
        void unRegisterRenderer(const shared_ptr<Renderer>& renderer);

        [[nodiscard]] VkImageView createImageView(VkImage image,
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
        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        static void transitionImageLayout(VkCommandBuffer commandBuffer,
                                   VkImage image,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout,
                                   VkAccessFlags srcAccessMask,
                                   VkAccessFlags dstAccessMask,
                                   VkPipelineStageFlags srcStageMask,
                                   VkPipelineStageFlags dstStageMask,
                                   VkImageAspectFlags aspectMask,
                                   uint32_t mipLevels = 1) ;
        // Returns true if a given format support LINEAR filtering
        [[nodiscard]] VkBool32 formatIsFilterable(VkFormat format, VkImageTiling tiling) const;
        // Find a suitable IMAGE_TILING format (for the Depth buffering image)
        [[nodiscard]] VkFormat findImageTilingSupportedFormat(const vector<VkFormat>& candidates,
                                                VkImageTiling tiling,
                                                VkFormatFeatureFlags features) const;
        [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;
        void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    private:
        const Window& window;
        VkInstance vkInstance;
        list<shared_ptr<Renderer>> renderers;

        // Physical & logical device management
        VkSurfaceKHR surface;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkCommandPool commandPool;
        VkPhysicalDeviceProperties2 deviceProperties {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 
        };
        VkPhysicalDeviceIDProperties physDeviceIDProps {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES 
        };
        VkSampleCountFlagBits samples;

        // Total video memory given by the OS (not Vulkan)
        uint64_t dedicatedVideoMemory;
        // GPU description given by the OS (not Vulkan)
        string adapterDescription;
#ifdef _WIN32
        IDXGIAdapter3* dxgiAdapter{nullptr};
#endif

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
        [[nodiscard]] static bool checkDeviceExtensionSupport(VkPhysicalDevice vkPhysicalDevice, const vector<const char*>& deviceExtensions);
        // Get the swap chain format, default for sRGB/NON-LINEAR
        [[nodiscard]] static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
        // Get the swap chain present mode, default to MAILBOX, if not avaible FIFO (V-SYNC)
        [[nodiscard]] static VkPresentModeKHR chooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresentModes);

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

        // Set the initiale state for the dynamic rendering
        void setInitialState(VkCommandBuffer commandBuffer);
        // Rate physical device by properties to find the best suitable GPU
        [[nodiscard]] uint32_t rateDeviceSuitability(VkPhysicalDevice vkPhysicalDevice, const vector<const char*>& deviceExtensions) const;
        // Get the swap chain capabilities
        [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice vkPhysicalDevice) const;
        // Get the supported queues families for a particular GPU
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice vkPhysicalDevice) const;
        // Get the maximum MSAA samples
        [[nodiscard]] VkSampleCountFlagBits getMaxUsableMSAASampleCount() const;
        // Get the swap chain images sizes
        [[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
        // Get the GPU details from the OS
        void getAdapterDescFromOS();

    public:
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
    };

}