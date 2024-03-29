#include "z0/vulkan/vulkan_instance.hpp"
#include "z0/helpers/window_helper.hpp" // needed for GLFW support
#include "z0/log.hpp"

#include <cstring>

namespace z0 {

    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
    // https://github.com/zeux/volk
    VulkanInstance::VulkanInstance() {
        if (volkInitialize() != VK_SUCCESS) {
            die("Failed to initialize Volk");
        }
#ifdef GLFW_VERSION_MAJOR
        if (!glfwInit() || !glfwVulkanSupported()) {
            die("Failed to initialize GLFW");
        }
#endif

        // Check if all the requested Vulkan layers are supported by the Vulkan instance
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char* layerName : requestedLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                die("A requested Vulkan layer is not supported");
            }
        }

        std::vector<const char*> instanceExtensions{};
#ifdef GLFW_VERSION_MAJOR
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        for (int i = 0; i < glfwExtensionCount; i++)  {
            instanceExtensions.push_back(glfwExtensions[i]);
        }
#endif
        // for Vulkan Memory Allocator
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

        const VkApplicationInfo applicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = VK_API_VERSION_1_3
        };
        const VkInstanceCreateInfo createInfo = {
                VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                nullptr,
                0,
                &applicationInfo,
                static_cast<uint32_t>(requestedLayers.size()),
                requestedLayers.data(),
                static_cast<uint32_t>(instanceExtensions.size()),
                instanceExtensions.data()
        };
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            die("Failed to create Vulkan instance");
        }
        volkLoadInstance(instance);
    }

    VulkanInstance::~VulkanInstance() {
        vkDestroyInstance(instance, nullptr);
    }

}