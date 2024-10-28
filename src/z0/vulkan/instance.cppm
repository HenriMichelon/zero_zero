module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include "z0/libraries.h"

export module z0:Instance;

import :ApplicationConfig;
import :Device;
import :Window;

export namespace z0 {

    /**
     * Vulkan [VkInstance](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkInstance.html) helper.
     */
    class Instance {
    public:
        explicit Instance();
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
