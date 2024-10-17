module;
#include "z0/libraries.h"
#include <volk.h>

export module z0:SkyboxRenderer;

import :Constants;
import :Renderpass;
import :Device;
import :Cubemap;
import :Environment;
import :Camera;
import :Buffer;
import :Descriptors;

export namespace z0 {

    class SkyboxRenderer : public Renderpass {
    public:
        SkyboxRenderer(Device &device, const string &shaderDirectory, VkClearValue clearColor);

        void loadScene(const shared_ptr<Cubemap> &_cubemap, uint32_t currentFrame);

        void cleanup() override;

        void update(Camera *currentCamera, const Environment *currentEnvironment, uint32_t currentFrame);

        void loadShaders() override;

        void createDescriptorSetLayout() override;

        void createOrUpdateDescriptorSet(bool create) override;

        void recordCommands(VkCommandBuffer commandBuffer, uint32_t currentFrame) override;

    private:
        struct GobalUniformBuffer {
            mat4 projection{1.0f};
            mat4 view{1.0f};
            vec4 ambient{1.0f, 1.0f, 1.0f, 1.0f}; // RGB + Intensity;
        };

        uint32_t            vertexCount;
        unique_ptr<Buffer>  vertexBuffer;
        vector<shared_ptr<Cubemap>> cubemap{MAX_FRAMES_IN_FLIGHT};
    };

}
