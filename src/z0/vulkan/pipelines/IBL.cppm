/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <volk.h>
#include "z0/libraries.h"

export module z0.IBLPipeline;

import z0.Image;
import z0.Cubemap;

import z0.Device;
import z0.ComputePipeline;
import z0.Descriptors;
import z0.VulkanImage;
import z0.VulkanCubemap;

export namespace z0 {

    /*
     * Pipeline to converts equirectangular projection texture into a cubemap.
     */
    class IBLPipeline : public ComputePipeline {
    public:
        explicit IBLPipeline(Device &device);
        ~IBLPipeline() override;

        void convert(const shared_ptr<VulkanImage>&   hdrFile,
                     const shared_ptr<VulkanCubemap>& cubemap) const;
        void preComputeSpecular(const shared_ptr<VulkanCubemap>& unfilteredCubemap, const shared_ptr<VulkanCubemap>& cubemap) const;
        void preComputeIrradiance(const shared_ptr<VulkanCubemap>& cubemap, const shared_ptr<VulkanCubemap>& irradianceCubemap) const;
        void preComputeBRDF(const shared_ptr<VulkanImage>& brdfLut) const;

    private:
        struct SpecularFilterPushConstants {
            alignas(4) uint32_t level;
            alignas(4) float roughness;
        };

        unique_ptr<DescriptorPool> descriptorPool;
        unique_ptr<DescriptorSetLayout> descriptorSetLayout;
        VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
        VkSampler computeSampler{VK_NULL_HANDLE};

        static constexpr auto BINDING_INPUT_TEXTURE{0};
        static constexpr auto BINDING_OUTPUT_TEXTURE{1};
        static constexpr auto BINDING_OUTPUT_MIPMAPS{2};

        static constexpr auto specializationMap = VkSpecializationMapEntry { 0, 0, sizeof(uint32_t) };
        static constexpr uint32_t specializationData[]{ EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1 };
        static constexpr auto specializationInfo = VkSpecializationInfo{ 1, &specializationMap, sizeof(specializationData), specializationData };
    };

}
