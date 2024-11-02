/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>

export module z0:ComputePipeline;

import :Device;
import :Pipeline;

export namespace z0 {

    /*
     * Base class for all compute pipelines
     */
    class ComputePipeline : public Pipeline {
    public:
        explicit ComputePipeline(Device &device);
        ~ComputePipeline() override = default;

    protected:
        VkPipeline createPipeline(VkShaderModule shader, const VkSpecializationInfo* specializationInfo = nullptr) const;
    };

}
