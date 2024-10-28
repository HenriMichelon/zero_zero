module;
#ifdef _WIN32
#include <windows.h>
#include <dxgi1_4.h>
#endif
#include "z0/libraries.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include "vk_mem_alloc.h"

module z0;

import :ApplicationConfig;
import :Window;
import :Renderer;
import :Tools;
import :Device;

namespace z0 {
    Device::Device(VkInstance                  instance,
                   const vector<const char *> &requestedLayers,
                   const ApplicationConfig &   applicationConfig,
                   const Window &              theWindow):
        framesInFlight{applicationConfig.framesInFlight},
        samples{static_cast<VkSampleCountFlagBits>(applicationConfig.msaa)},
        vkInstance{instance},
        window{theWindow} {
        commandBuffers.resize(framesInFlight);
        imageAvailableSemaphores.resize(framesInFlight);
        renderFinishedSemaphores.resize(framesInFlight);
        inFlightFences.resize(framesInFlight);

        //////////////////// Find the best GPU
        // Check for at least one supported Vulkan physical device
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Selecting-a-physical-device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
            die("Failed to find GPUs with Vulkan support");

        // Get a VkSurface for drawing in the window, must be done before picking the better physical device
        // since we need the VkSurface for vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface#page_Window-surface-creation
#ifdef _WIN32
        const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                .hinstance = GetModuleHandle(nullptr),
                .hwnd = theWindow._getHandle(),
        };
        if (vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS)
            die("Failed to create window surface!");
#endif

        // Requested device extensions
        const vector deviceExtensions = {
                // Mandatory to create a swap chain
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                // https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering/README.html
                VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
                // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
                VK_EXT_SHADER_OBJECT_EXTENSION_NAME,
                // for Vulkan Memory Allocator
                VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
#ifndef NDEBUG
                // To debugPrintEXT() from shaders :-)
                // See shader_debug_env.cmd for setup with environment variables
                VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
#endif
        };

        // Use the better Vulkan physical device found
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
        vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
        // Use an ordered map to automatically sort candidates by increasing score
        multimap<uint32_t, VkPhysicalDevice> candidates;
        for (const auto &dev : devices) {
            uint32_t score = rateDeviceSuitability(dev, deviceExtensions);
            candidates.insert(make_pair(score, dev));
        }
        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            // Select the better suitable device found
            physicalDevice = candidates.rbegin()->second;
            // Select the best MSAA samples count if requested
            if (applicationConfig.msaa == MSAA_AUTO)
                samples = getMaxUsableMSAASampleCount();
            deviceProperties.pNext = &physDeviceIDProps;
            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties);
            // Get the GPU description and total memory
            getAdapterDescFromOS();
        } else { die("Failed to find a suitable GPU!"); }

        //////////////////// Create Vulkan device

        // Find a graphical command queue and a presentation command queue
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Queue-families
        vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        QueueFamilyIndices              indices = findQueueFamilies(physicalDevice);
        {
            auto queuePriority       = 1.0f;
            set  uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            for (auto queueFamily : uniqueQueueFamilies) {
                const VkDeviceQueueCreateInfo queueCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueFamilyIndex = queueFamily,
                        .queueCount = 1,
                        .pQueuePriorities = &queuePriority,
                };
                queueCreateInfos.push_back(queueCreateInfo);
            }
        }

        // Initialize device extensions and create a logical device
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Specifying-used-device-features
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Creating-the-logical-device
        {
            // https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html
            VkPhysicalDeviceShaderObjectFeaturesEXT deviceShaderObjectFeatures{
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
                    .pNext = VK_NULL_HANDLE,
                    .shaderObject = VK_TRUE,
            };
            // https://lesleylai.info/en/vk-khr-dynamic-rendering/
            const VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{
                    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
                    .pNext = &deviceShaderObjectFeatures,
                    .dynamicRendering = VK_TRUE,
            };
            constexpr VkPhysicalDeviceFeatures deviceFeatures{
                    .samplerAnisotropy = VK_TRUE
            };
            const VkDeviceCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = &dynamicRenderingFeature,
                    .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
                    .pQueueCreateInfos = queueCreateInfos.data(),
                    .enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
                    .ppEnabledLayerNames = requestedLayers.data(),
                    .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
                    .ppEnabledExtensionNames = deviceExtensions.data(),
                    .pEnabledFeatures = &deviceFeatures,
            };
            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
                die("Failed to create logical device!");
        }

        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues#page_Retrieving-queue-handles
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface#page_Creating-the-presentation-queue
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        ////////////////////  Create the device command pool

        // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Command_buffers#page_Command-pools
        auto                          queueFamilyIndices = findQueueFamilies(physicalDevice);
        const VkCommandPoolCreateInfo poolInfo           = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
        };
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
            die("Failed to create the command pool");

        //////////////////// Create VMA allocator

        // https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html
        const VmaVulkanFunctions vulkanFunctions{
                .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                .vkAllocateMemory = vkAllocateMemory,
                .vkFreeMemory = vkFreeMemory,
                .vkMapMemory = vkMapMemory,
                .vkUnmapMemory = vkUnmapMemory,
                .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
                .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
                .vkBindBufferMemory = vkBindBufferMemory,
                .vkBindImageMemory = vkBindImageMemory,
                .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
                .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
                .vkCreateBuffer = vkCreateBuffer,
                .vkDestroyBuffer = vkDestroyBuffer,
                .vkCreateImage = vkCreateImage,
                .vkDestroyImage = vkDestroyImage,
                .vkCmdCopyBuffer = vkCmdCopyBuffer,
                .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
                .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR,
                .vkBindBufferMemory2KHR = vkBindBufferMemory2KHR,
                .vkBindImageMemory2KHR = vkBindImageMemory2KHR,
                .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR,
                .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
                .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
        };
        const VmaAllocatorCreateInfo allocatorInfo = {
                .physicalDevice = physicalDevice,
                .device = device,
                .pVulkanFunctions = &vulkanFunctions,
                .instance = vkInstance,
                .vulkanApiVersion = deviceProperties.properties.apiVersion,
        };
        vmaCreateAllocator(&allocatorInfo, &allocator);

        //////////////////// Create swap chain

        createSwapChain();

        //////////////////// Create command buffers

        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Command_buffers
        {
            const VkCommandBufferAllocateInfo allocInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = commandPool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
            };
            if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
                die("failed to allocate command buffers!");
        }

        //////////////////// Create sync objects
        {
            constexpr VkSemaphoreCreateInfo semaphoreInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
            };
            constexpr VkFenceCreateInfo fenceInfo{
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT
            };
            for (size_t i = 0; i < framesInFlight; i++) {
                if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
                    ||
                    vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
                    ||
                    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                    die("failed to create semaphores!");
                }
            }
        }
    }

    void Device::cleanup() {
        for (const auto &renderer : renderers) { renderer->cleanup(); }
        renderers.clear();
        for (size_t i = 0; i < framesInFlight; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        cleanupSwapChain();
        vkDestroyCommandPool(device, commandPool, nullptr);
        vmaDestroyAllocator(allocator);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(vkInstance, surface, nullptr);
#ifdef _WIN32
        dxgiAdapter->Release();
#endif
    }

    void Device::drawFrame(const uint32_t currentFrame) {
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
        // wait until the GPU has finished rendering the last frame.
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        uint32_t imageIndex;
        auto     result = vkAcquireNextImageKHR(device,
                                                swapChain,
                                                UINT64_MAX,
                                                imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE,
                                                &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            for (const auto &renderer : renderers) { renderer->recreateImagesResources(); }
            return;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { die("failed to acquire swap chain image!"); }
        {
            vkResetCommandBuffer(commandBuffers[currentFrame], 0);
            for (const auto &renderer : renderers) { renderer->update(currentFrame); }
            constexpr VkCommandBufferBeginInfo beginInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = 0,
                    .pInheritanceInfo = nullptr
            };
            if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
                die("failed to begin recording command buffer!");
            }

            setInitialState(commandBuffers[currentFrame]);
            const auto &lastRenderer = renderers.back();
            for (const auto &renderer : renderers) {
                renderer->beginRendering(commandBuffers[currentFrame], currentFrame);
                renderer->recordCommands(commandBuffers[currentFrame], currentFrame);
                renderer->endRendering(commandBuffers[currentFrame], currentFrame, renderer == lastRenderer);
            }

            transitionImageLayout(
                    commandBuffers[currentFrame],
                    swapChainImages[imageIndex],
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    0,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);
            vkCmdBlitImage(commandBuffers[currentFrame],
                           lastRenderer->getImage(currentFrame),
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           swapChainImages[imageIndex],
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &colorImageBlit,
                           VK_FILTER_LINEAR);
            transitionImageLayout(
                    commandBuffers[currentFrame],
                    swapChainImages[imageIndex],
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                    0,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT);

            if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
                die("failed to record command buffer!");
            }
        }

        const VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        {
            const VkSemaphore              waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            constexpr VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            const VkSubmitInfo             submitInfo{
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = waitSemaphores,
                    .pWaitDstStageMask = waitStages,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &commandBuffers[currentFrame],
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = signalSemaphores
            };
            if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                die("failed to submit draw command buffer!");
            }
            // vkQueueWaitIdle(graphicsQueue);
        }

        {
            const VkSwapchainKHR   swapChains[] = {swapChain};
            const VkPresentInfoKHR presentInfo{
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = signalSemaphores,
                    .swapchainCount = 1,
                    .pSwapchains = swapChains,
                    .pImageIndices = &imageIndex,
                    .pResults = nullptr // Optional
            };
            result = vkQueuePresentKHR(presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
                recreateSwapChain();
                for (const auto &renderer : renderers) { renderer->recreateImagesResources(); }
            } else if (result != VK_SUCCESS) { die("failed to present swap chain image!"); }
        }

    }

    void Device::wait() const {
        for(auto i = 0; i < getFramesInFlight(); i++) {
            vkWaitForFences(device, 1, &inFlightFences[i], VK_TRUE, UINT64_MAX);
        }
        vkQueueWaitIdle(graphicsQueue);
        vkDeviceWaitIdle(device);
    }

    void Device::registerRenderer(const shared_ptr<Renderer> &renderer) { renderers.push_front(renderer); }

    void Device::unRegisterRenderer(const shared_ptr<Renderer> &renderer) { renderers.remove(renderer); }

    VkImageView Device::createImageView(const VkImage            image,
                                        const VkFormat           format,
                                        const VkImageAspectFlags aspectFlags,
                                        const uint32_t           mipLevels,
                                        const VkImageViewType    type,
                                        const uint32_t           baseArrayLayer,
                                        const uint32_t           layers,
                                        const uint32_t           baseMipLevel) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Image_views
        const VkImageViewCreateInfo viewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = image,
                .viewType = type,
                .format = format,
                .subresourceRange = {
                        .aspectMask = aspectFlags,
                        .baseMipLevel = baseMipLevel,
                        .levelCount = mipLevels,
                        .baseArrayLayer = baseArrayLayer,
                        // Note : VK_REMAINING_ARRAY_LAYERS does not work for VK_IMAGE_VIEW_TYPE_2D_ARRAY
                        // we have to specify the exact number of layers or texture() only read the first layer
                        .layerCount = type == VK_IMAGE_VIEW_TYPE_CUBE ? VK_REMAINING_ARRAY_LAYERS : layers
                }
        };
        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            die("failed to create texture image view!");
        return imageView;
    }

    void Device::createImage(const uint32_t              width,
                             const uint32_t              height,
                             const uint32_t              mipLevels,
                             const VkSampleCountFlagBits numSamples,
                             const VkFormat              format,
                             const VkImageTiling         tiling,
                             const VkImageUsageFlags     usage,
                             const VkMemoryPropertyFlags properties,
                             VkImage &                   image,
                             VkDeviceMemory &            imageMemory,
                             const VkImageCreateFlags    flags,
                             const uint32_t              layers) const {
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Texture-Image
        const VkImageCreateInfo imageInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .flags = flags,
                .imageType =  VK_IMAGE_TYPE_2D,
                .format = format,
                .extent = {width, height, 1},
                .mipLevels = mipLevels,
                .arrayLayers = layers,
                .samples = numSamples,
                .tiling = tiling,
                .usage = usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) { die("failed to create image!"); }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        const VkMemoryAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memRequirements.size,
                .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)
        };
        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            die("failed to allocate image memory!");
        }
        if (vkBindImageMemory(device, image, imageMemory, 0) != VK_SUCCESS) {
            die("failed to bind image memory!");
        }
    }

    uint32_t Device::findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { return i; }
        }
        die("failed to find suitable memory type!");
        return 0;
    }

    void Device::transitionImageLayout(const VkCommandBuffer      commandBuffer,
                                       const VkImage              image,
                                       const VkImageLayout        oldLayout,
                                       const VkImageLayout        newLayout,
                                       const VkAccessFlags        srcAccessMask,
                                       const VkAccessFlags        dstAccessMask,
                                       const VkPipelineStageFlags srcStageMask,
                                       const VkPipelineStageFlags dstStageMask,
                                       const VkImageAspectFlags   aspectMask,
                                       const uint32_t             mipLevels) {
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
        // https://vulkan-tutorial.com/Generating_Mipmaps#page_Generating-Mipmaps
        const VkImageMemoryBarrier barrier = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = srcAccessMask,
                .dstAccessMask = dstAccessMask,
                .oldLayout = oldLayout,
                .newLayout = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image = image,
                .subresourceRange = {
                        .aspectMask = aspectMask,
                        .baseMipLevel = 0,
                        .levelCount = mipLevels,
                        .baseArrayLayer = 0,
                        .layerCount = VK_REMAINING_ARRAY_LAYERS,
                }
        };
        vkCmdPipelineBarrier(
                commandBuffer,
                srcStageMask,
                dstStageMask,
                0,
                0,
                nullptr,
                0,
                nullptr,
                1,
                &barrier);
    }

    // Returns true if a given format support LINEAR filtering
    VkBool32 Device::formatIsFilterable(const VkFormat format, const VkImageTiling tiling) const {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
        if (tiling == VK_IMAGE_TILING_OPTIMAL)
            return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        if (tiling == VK_IMAGE_TILING_LINEAR)
            return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
        return false;
    }

    VkFormat Device::findImageSupportedFormat(const vector<VkFormat> &   candidates,
                                              const VkImageTiling        tiling,
                                              const VkFormatFeatureFlags features) const {
        // https://vulkan-tutorial.com/Depth_buffering#page_Depth-image-and-view
        for (const auto format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        die("failed to find supported format");
        return candidates.at(0);
    }

    VkCommandBuffer Device::beginOneTimeCommandBuffer() const {
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
        const VkCommandBufferAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1
        };
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        constexpr VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void Device::endOneTimeCommandBuffer(VkCommandBuffer commandBuffer) const {
        // https://vulkan-tutorial.com/Texture_mapping/Images#page_Layout-transitions
        vkEndCommandBuffer(commandBuffer);
        const VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer
        };
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    void Device::createSwapChain() {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        const VkSurfaceFormatKHR      surfaceFormat    = chooseSwapSurfaceFormat(swapChainSupport.formats);
        const VkPresentModeKHR        presentMode      = chooseSwapPresentMode(swapChainSupport.presentModes);
        const VkExtent2D              extent           = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        {
            VkSwapchainCreateInfoKHR createInfo = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface = surface,
                    .minImageCount = imageCount,
                    .imageFormat = surfaceFormat.format,
                    .imageColorSpace = surfaceFormat.colorSpace,
                    .imageExtent = extent,
                    .imageArrayLayers = 1,
                    // VK_IMAGE_USAGE_TRANSFER_DST_BIT for Blit or Revolve (see presentToSwapChain())
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    .preTransform = swapChainSupport.capabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = presentMode,
                    .clipped = VK_TRUE
            };
            const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
            const uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices   = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0; // Optional
                createInfo.pQueueFamilyIndices   = nullptr; // Optional
            }
            createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : *oldSwapChain;
            // Need VK_KHR_SWAPCHAIN extension or it will crash (no validation error)
            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
                die("Failed to create Vulkan swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent      = extent;

        swapChainImageViews.resize(swapChainImages.size());
        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = createImageView(swapChainImages[i],
                                                     swapChainImageFormat,
                                                     VK_IMAGE_ASPECT_COLOR_BIT,
                                                     1);
        }

        // For bliting image to swapchain
        constexpr VkOffset3D vkOffset0{0, 0, 0};
        const VkOffset3D     vkOffset1{
                static_cast<int32_t>(swapChainExtent.width),
                static_cast<int32_t>(swapChainExtent.height),
                1,
        };
        colorImageBlit.srcOffsets[0]                 = vkOffset0;
        colorImageBlit.srcOffsets[1]                 = vkOffset1;
        colorImageBlit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.srcSubresource.mipLevel       = 0;
        colorImageBlit.srcSubresource.baseArrayLayer = 0;
        colorImageBlit.srcSubresource.layerCount     = 1;
        colorImageBlit.dstOffsets[0]                 = vkOffset0;
        colorImageBlit.dstOffsets[1]                 = vkOffset1;
        colorImageBlit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageBlit.dstSubresource.mipLevel       = 0;
        colorImageBlit.dstSubresource.baseArrayLayer = 0;
        colorImageBlit.dstSubresource.layerCount     = 1;
    }

    void Device::cleanupSwapChain() const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation#page_Recreating-the-swap-chain
        for (auto &swapChainImageView : swapChainImageViews) {
            vkDestroyImageView(device, swapChainImageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    void Device::recreateSwapChain() {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Swap_chain_recreation
        // TODO wait for all Window events to be processed

        vkDeviceWaitIdle(device);
        cleanupSwapChain();
        createSwapChain();
    }

    bool Device::checkDeviceExtensionSupport(VkPhysicalDevice            vkPhysicalDevice,
                                             const vector<const char *> &deviceExtensions) {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Checking-for-swap-chain-support
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);
        vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(vkPhysicalDevice,
                                             nullptr,
                                             &extensionCount,
                                             availableExtensions.data());
        set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto &extension : availableExtensions) { requiredExtensions.erase(extension.extensionName); }
        return requiredExtensions.empty();
    }

    VkSurfaceFormatKHR Device::chooseSwapSurfaceFormat(
            const vector<VkSurfaceFormatKHR> &availableFormats) {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Choosing-the-right-settings-for-the-swap-chain
        for (const auto &availableFormat : availableFormats) {
            // Using sRGB no-linear color space
            // https://learnopengl.com/Advanced-Lighting/Gamma-Correction
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { return availableFormat; }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR Device::chooseSwapPresentMode(
            const vector<VkPresentModeKHR> &availablePresentModes) {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Presentation-mode
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    void Device::setInitialState(const VkCommandBuffer commandBuffer) const {
        // https://github.com/KhronosGroup/Vulkan-Samples/blob/main/samples/extensions/shader_object/shader_object.cpp
        vkCmdSetRasterizerDiscardEnable(commandBuffer, VK_FALSE);
        constexpr VkColorBlendEquationEXT colorBlendEquation{
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
        };
        vkCmdSetColorBlendEquationEXT(commandBuffer, 0, 1, &colorBlendEquation);

        vkCmdSetPrimitiveTopologyEXT(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        vkCmdSetPrimitiveRestartEnableEXT(commandBuffer, VK_FALSE);
        constexpr VkSampleMask sample_mask = 0xffffffff;
        vkCmdSetSampleMaskEXT(commandBuffer, samples, &sample_mask);
        vkCmdSetPolygonModeEXT(commandBuffer, VK_POLYGON_MODE_FILL);
        vkCmdSetFrontFace(commandBuffer, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        vkCmdSetDepthCompareOp(commandBuffer, VK_COMPARE_OP_LESS_OR_EQUAL);
        vkCmdSetStencilTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetLogicOpEnableEXT(commandBuffer, VK_FALSE);
        vkCmdSetDepthBiasEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        vkCmdSetDepthWriteEnable(commandBuffer, VK_FALSE);

        // Use RGBA color write mask
        constexpr VkColorComponentFlags color_component_flags[] = {
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_A_BIT
        };
        vkCmdSetColorWriteMaskEXT(commandBuffer, 0, 1, color_component_flags);
    }

    uint32_t Device::rateDeviceSuitability(VkPhysicalDevice            vkPhysicalDevice,
                                           const vector<const char *> &deviceExtensions) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
        VkPhysicalDeviceProperties _deviceProperties;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &_deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);

        uint32_t score = 0;
        // Discrete GPUs have a significant performance advantage
        if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;
        // Maximum possible size of textures affects graphics quality
        score += _deviceProperties.limits.maxImageDimension2D;
        // Application can't function without geometry shaders
        if (!deviceFeatures.geometryShader)
            return 0;

        bool extensionsSupported = checkDeviceExtensionSupport(vkPhysicalDevice, deviceExtensions);
        bool swapChainAdequate   = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkPhysicalDevice);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice);
        if ((!extensionsSupported) || (!indices.isComplete()) || (!swapChainAdequate))
            return 0;
        return score;
    }

    Device::SwapChainSupportDetails Device::querySwapChainSupport(const VkPhysicalDevice vkPhysicalDevice) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Querying-details-of-swap-chain-support
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice,
                                                      surface,
                                                      &presentModeCount,
                                                      details.presentModes.data());
        }
        return details;
    }

    Device::QueueFamilyIndices Device::findQueueFamilies(const VkPhysicalDevice vkPhysicalDevice) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Queue-families
        QueueFamilyIndices indices;
        uint32_t           queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, surface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;
            if (indices.isComplete())
                break;
            i++;
        }
        return indices;
    }

    VkSampleCountFlagBits Device::getMaxUsableMSAASampleCount() const {
        // https://vulkan-tutorial.com/Multisampling#page_Getting-available-sample-count
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
        die("No MSAA support");
        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkExtent2D Device::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain#page_Swap-extent
        if (capabilities.currentExtent.width != numeric_limits<uint32_t>::max()) { return capabilities.currentExtent; }
        VkExtent2D actualExtent{
                .width = window.getWidth(),
                .height = window.getHeight()
        };
        actualExtent.width = std::max(
                capabilities.minImageExtent.width,
                std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
                capabilities.minImageExtent.height,
                std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }


#ifdef _WIN32
    // https://dev.to/reg__/there-is-a-way-to-query-gpu-memory-usage-in-vulkan---use-dxgi-1f0d
    void Device::getAdapterDescFromOS() {
        IDXGIFactory4 *dxgiFactory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
        IDXGIAdapter1 *tmpDxgiAdapter = nullptr;
        UINT           adapterIndex   = 0;
        while (dxgiFactory->EnumAdapters1(adapterIndex, &tmpDxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 desc;
            tmpDxgiAdapter->GetDesc1(&desc);
            if (memcmp(&desc.AdapterLuid, physDeviceIDProps.deviceLUID, VK_LUID_SIZE) == 0) {
                tmpDxgiAdapter->QueryInterface(IID_PPV_ARGS(&dxgiAdapter));
                adapterDescription   = wstring_to_string(desc.Description);
                dedicatedVideoMemory = desc.DedicatedVideoMemory;
            }
            tmpDxgiAdapter->Release();
            ++adapterIndex;
        }
        dxgiFactory->Release();
        log(adapterDescription + " VRAM", to_string(dedicatedVideoMemory / 1024 / 1024) + "Mb");
    }

    uint64_t Device::getVideoMemoryUsage() const {
        DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
        dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info);
        return info.CurrentUsage;
    }
#endif
}
