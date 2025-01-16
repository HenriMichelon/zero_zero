/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"
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
        Renderer(bool canBeThreaded);
        virtual ~Renderer();

        // Returns the offscreen image buffer
        [[nodiscard]] virtual VkImage getImage(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Returns the offscreen image buffer view
        [[nodiscard]] virtual VkImageView getImageView(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Release all the Vulkan resources
        virtual void cleanup() = 0;

        // Update frame data
        virtual void update(uint32_t currentFrame) {}

        // Render one frame
        virtual void drawFrame(uint32_t currentFrame, bool isLast) = 0;

        // Create images, image views & buffers
        virtual void createImagesResources() {}

        // Release images, image views & buffers
        virtual void cleanupImagesResources() {}

        // Release and re-create images, image views & buffers, in needed
        virtual void recreateImagesResources() {}

        virtual vector<VkCommandBuffer> getCommandBuffers(uint32_t currentFrame) const;

        inline bool canBeThreaded() const { return threaded; }

    protected:
        vector<VkCommandPool> commandPools;
        vector<VkCommandBuffer> commandBuffers;
    private:
        bool threaded;
    };

}
