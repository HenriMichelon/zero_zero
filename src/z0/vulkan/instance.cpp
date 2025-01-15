/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <cassert>
#include "z0/libraries.h"

module z0.vulkan.Instance;

import z0.ApplicationConfig;
import z0.Tools;
import z0.Window;

import z0.vulkan.Device;

namespace z0 {

#ifndef NDEBUG
    // Redirect the Vulkan validation layers to the logging system
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                        VkDebugUtilsMessageTypeFlagsEXT,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *) {
        log("validation layer: ", pCallbackData->pMessage);
        return VK_FALSE;
    }

    // vkCreateDebugUtilsMessengerEXT linker
    VkResult CreateDebugUtilsMessengerEXT(const VkInstance                          instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks              *pAllocator,
                                          VkDebugUtilsMessengerEXT                 *pDebugMessenger) {
        const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // vkDestroyDebugUtilsMessengerEXT linker
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator) {
        const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

#endif

    Instance::Instance() {
        // https://github.com/zeux/volk
        if (volkInitialize() != VK_SUCCESS) { die("Failed to initialize Volk"); }

        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
#ifndef NDEBUG
        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
#endif

        // Check if all the requested Vulkan layers are supported by the Vulkan instance
#ifndef NDEBUG
        requestedLayers.push_back(validationLayerName);
#endif
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        for (const char *layerName : requestedLayers) {
            bool layerFound = false;
            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) die("A requested Vulkan layer is not supported");
        }

        vector<const char *> instanceExtensions{};
        // Abstract native platform surface or window objects for use with Vulkan.
        instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
        // Provides a mechanism to create a VkSurfaceKHR object (defined by the VK_KHR_surface extension)
        // that refers to a Win32 HWND
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
        // For Vulkan Memory Allocator
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifndef NDEBUG
        // To use a debug callback for validation message
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // https://docs.vulkan.org/samples/latest/samples/extensions/shader_debugprintf/README.html
        // Use run/debug config with environment variables (cf. shader_debug_env.cmd)
        /*instanceExtensions.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);
        const char *setting_debug_action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
        const char *setting_validate_gpu_based[] = {"GPU_BASED_DEBUG_PRINTF"};
        constexpr VkBool32 setting_enable_printf_to_stdout{VK_TRUE};

        const auto layerSettings = array{
                VkLayerSettingEXT{validationLayerName, "debug_action",
                    VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_debug_action},
                VkLayerSettingEXT{validationLayerName, "validate_gpu_based",
                    VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_validate_gpu_based},
                VkLayerSettingEXT{validationLayerName, "printf_to_stdout",
                    VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_printf_to_stdout},
        };
        const auto layerSettingsCreateInfo = VkLayerSettingsCreateInfoEXT {
            .sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
            .pNext = nullptr,
            .settingCount = layerSettings.size(),
            .pSettings = layerSettings.data()
        };*/

#endif

        // Use Vulkan 1.4.x
        constexpr VkApplicationInfo applicationInfo{.sType      = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                                    .apiVersion = VK_API_VERSION_1_4};
        // Initialize Vulkan instances, extensions & layers
        const VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                                 nullptr,
                                                 0,
                                                 &applicationInfo,
                                                 static_cast<uint32_t>(requestedLayers.size()),
                                                 requestedLayers.data(),
                                                 static_cast<uint32_t>(instanceExtensions.size()),
                                                 instanceExtensions.data()};
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            die("Failed to create Vulkan instance");
        }
        volkLoadInstance(instance);

#ifndef NDEBUG
        // Initialize validating layer for logging
        constexpr VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
                .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
                .pUserData       = nullptr,
        };
        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            die("failed to set up debug messenger!");
        }
#endif
    }

    unique_ptr<Device> Instance::createDevice(
            const ApplicationConfig &applicationConfig,
            const Window& window) const {
        return make_unique<Device>(instance, requestedLayers, applicationConfig, window);
    }

    Instance::~Instance() {
#ifndef NDEBUG
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
        vkDestroyInstance(instance, nullptr);
    }

} // namespace z0
