#pragma once

#include "z0/framebuffers/depth_frame_buffer.h"
#include "z0/renderers/base_renderpass.h"
#include "z0/renderers/base_renderer.h"
#include "z0/nodes/mesh_instance.h"
#include "z0/nodes/camera.h"

namespace z0 {

    class BaseModelsRenderer: public BaseRenderpass, public BaseRenderer {
    public:
        virtual void addNode(const shared_ptr<Node>& node);
        virtual void removeNode(const shared_ptr<Node>& node);
        void cleanup() override;

    protected:
        Camera* currentCamera {nullptr};
        list<MeshInstance*> models {};
        vector<unique_ptr<Buffer>> modelUniformBuffers{MAX_FRAMES_IN_FLIGHT};
        shared_ptr<DepthFrameBuffer> depthFrameBuffer;

        BaseModelsRenderer(const Device& device, const string& shaderDirectory);

        virtual void addingModel(MeshInstance* meshInstance, uint32_t index) {};
        virtual void removingModel(MeshInstance* meshInstance) {};
        void setInitialState(VkCommandBuffer commandBuffer);

    public:
        BaseModelsRenderer(const BaseModelsRenderer&) = delete;
        BaseModelsRenderer &operator=(const BaseModelsRenderer&) = delete;
    };

}