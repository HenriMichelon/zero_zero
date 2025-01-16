/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/vulkan.h"

export module z0.vulkan.ComputePipeline;

import z0.vulkan.Device;
import z0.vulkan.Pipeline;

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
