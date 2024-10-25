module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:Equirect2CubemapPipeline;

import :Constants;
import :Renderer;
import :Renderpass;
import :Device;
import :ColorFrameBufferHDR;
import :Descriptors;
import :Image;
import :Cubemap;

export namespace z0 {

    /**
     * Converts equirectangular projection texture into a cubemap.
     */
    class Equirect2CubemapPipeline {
    public:
        Equirect2CubemapPipeline(Device &device);
        ~Equirect2CubemapPipeline();

        void convert(const shared_ptr<Image>&   hdrFile,
                     const shared_ptr<Cubemap>& cubemap) const;
    private:
        Device &device;
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        unique_ptr<DescriptorPool> descriptorPool;
        unique_ptr<DescriptorSetLayout> descriptorSetLayout;

        vector<char> readFile(const string &fileName) const;

        VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) const;

        VkShaderModule createShaderModule(const vector<char>& code) const;

        VkPipeline createPipeline(VkPipelineLayout layout, VkShaderModule shader) const;
    };

}
