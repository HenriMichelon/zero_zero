module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:ComputePipeline;

import :Device;
import :Pipeline;

export namespace z0 {

    /**
     * Base class for all compute pipelines
     */
    class ComputePipeline : public Pipeline {
    public:
        explicit ComputePipeline(Device &device);
        ~ComputePipeline() override = default;

    protected:
        VkPipeline createPipeline(VkShaderModule shader) const;
    };

}
