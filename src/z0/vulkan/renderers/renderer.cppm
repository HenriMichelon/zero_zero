/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.vulkan.Renderer;

export namespace z0 {

    /*
     * Common interface of all renderers
     */
    class Renderer {
    public:
        Renderer(Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer();
        virtual ~Renderer();

        // Returns the offscreen image buffer
        [[nodiscard]] virtual VkImage getImage(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Returns the offscreen image buffer view
        [[nodiscard]] virtual VkImageView getImageView(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Release all the Vulkan resources
        virtual void cleanup() = 0;

        // Update frame data
        virtual void update(uint32_t currentFrame) {};

        // Start rendering one frame
        virtual void beginRendering(uint32_t currentFrame) {};

        // Render one frame
        virtual void recordCommands(uint32_t currentFrame) = 0;

        // End rendering one frame
        virtual void endRendering(uint32_t currentFrame, bool isLast) {};

        // Create images, image views & buffers
        virtual void createImagesResources() {};

        // Release images, image views & buffers
        virtual void cleanupImagesResources() {};

        // Release and re-create images, image views & buffers, in needed
        virtual void recreateImagesResources() {};

        inline VkCommandPool getCommandPool() const { return commandPool; }
        inline VkCommandBuffer getCommandBuffer(uint32_t currentFrame) const { return commandBuffers[currentFrame]; }

    private:
        VkCommandPool commandPool;
        vector<VkCommandBuffer> commandBuffers;
    };

}
