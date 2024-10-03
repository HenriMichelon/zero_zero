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
        [[nodiscard]] virtual VkImage getImage() const { return VK_NULL_HANDLE; }

        // Returns the offscreen image buffer view
        [[nodiscard]] virtual VkImageView getImageView() const { return VK_NULL_HANDLE; }

        // Release all the Vulkan resources
        virtual void cleanup() = 0;

        // Update frame datas
        virtual void update(uint32_t currentFrame) = 0;

        // Start rendering one frame
        virtual void beginRendering(VkCommandBuffer commandBuffer) = 0;

        // Render one frame
        virtual void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) = 0;

        // End rendering one frame
        virtual void endRendering(VkCommandBuffer commandBuffer, bool isLast) = 0;

        // Create images, imageviews & buffers
        virtual void createImagesResources() = 0;

        // Release images, imageviews & buffers
        virtual void cleanupImagesResources() = 0;

        // Release and re-create images, imageviews & buffers, in needed
        virtual void recreateImagesResources() = 0;
    };

}
