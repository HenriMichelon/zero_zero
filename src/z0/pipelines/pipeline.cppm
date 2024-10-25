module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:Pipeline;

import :Device;

export namespace z0 {

    /**
     * Base class for all pipelines
     */
    class Pipeline {
    public:
        explicit Pipeline(Device &device);
        virtual ~Pipeline();

    protected:
        Device &device;
        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};

        vector<char> readFile(const string &fileName) const;

        VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) const;

        VkShaderModule createShaderModule(const vector<char>& code) const;
    };

}
