module;
#include <volk.h>

export module z0:Renderer;

export namespace z0 {

    /**
     * Common interface of all renderers
     */
    class Renderer {
    public:
        virtual ~Renderer() = default;

        // Returns the offscreen image buffer
        [[nodiscard]] virtual VkImage getImage(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Returns the offscreen image buffer view
        [[nodiscard]] virtual VkImageView getImageView(uint32_t currentFrame) const { return VK_NULL_HANDLE; }

        // Release all the Vulkan resources
        virtual void cleanup() = 0;

        // Update frame data
        virtual void update(uint32_t currentFrame) {};

        // Start rendering one frame
        virtual void beginRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame) {};

        // Render one frame
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

        // End rendering one frame
        virtual void endRendering(VkCommandBuffer commandBuffer, uint32_t currentFrame, bool isLast) {};

        // Create images, image views & buffers
        virtual void createImagesResources() = 0;

        // Release images, image views & buffers
        virtual void cleanupImagesResources() = 0;

        // Release and re-create images, image views & buffers, in needed
        virtual void recreateImagesResources() = 0;
    };

}
