/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

export module z0.vulkan.Instance;

import z0.ApplicationConfig;
import z0.Window;

import z0.vulkan.Device;

export namespace z0 {

    /*
     * Vulkan [VkInstance](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html) helper.
     */
    class Instance {
    public:
        explicit Instance();
        Instance(Instance&&) = delete;
        Instance(Instance&) = delete;
        ~Instance();

        [[nodiscard]] inline  VkInstance getInstance() const { return instance; }

        [[nodiscard]] unique_ptr<Device> createDevice(
            const ApplicationConfig &applicationConfig,
            const Window& window) const;

    private:
        // The Vulkan global instance
        VkInstance instance{VK_NULL_HANDLE};
        // Used to redirect validation layers to the logging system
        VkDebugUtilsMessengerEXT debugMessenger;
        // All the vulkan layers used
        vector<const char *>requestedLayers{};
    };
}
