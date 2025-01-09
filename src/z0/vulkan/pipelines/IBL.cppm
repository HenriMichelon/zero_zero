/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.vulkan.IBLPipeline;

import z0.resources.Cubemap;
import z0.resources.Image;

import z0.vulkan.ComputePipeline;
import z0.vulkan.Descriptors;
import z0.vulkan.Device;
import z0.vulkan.Image;
import z0.vulkan.Cubemap;

export namespace z0 {

    /*
     * Pipeline to converts equirectangular projection texture into a cubemap.
     */
    class IBLPipeline : public ComputePipeline {
    public:
        explicit IBLPipeline(Device &device);
        ~IBLPipeline() override;

        void convert(const shared_ptr<VulkanImage>&    hdrFile,
                     const shared_ptr<VulkanCubemap>&  unfilteredCubemap,
                     const shared_ptr<VulkanCubemap>&  filteredCubemap,
                     const shared_ptr<VulkanCubemap>&  irradianceCubemap,
                     const shared_ptr<VulkanImage>&    brdfLut) const;

    private:
        struct SpecularFilterPushConstants {
            alignas(4) uint32_t level;
            alignas(4) float roughness;
        };

        unique_ptr<DescriptorPool>      descriptorPool;
        unique_ptr<DescriptorSetLayout> descriptorSetLayout;
        VkSampler                       computeSampler{VK_NULL_HANDLE};

        static constexpr auto BINDING_INPUT_TEXTURE{0};
        static constexpr auto BINDING_OUTPUT_TEXTURE{1};
        static constexpr auto BINDING_OUTPUT_MIPMAPS{2};

        static constexpr auto specializationMap = VkSpecializationMapEntry { 0, 0, sizeof(uint32_t) };
        static inline uint32_t specializationData[]{ static_cast<uint32_t>(EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1) };
        static constexpr auto specializationInfo = VkSpecializationInfo{ 1, &specializationMap, sizeof(specializationData), specializationData };
    };

}
