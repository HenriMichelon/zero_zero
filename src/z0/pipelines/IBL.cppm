module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:IBLPipeline;

import :Device;
import :ComputePipeline;
import :Descriptors;
import :Image;
import :Cubemap;

export namespace z0 {

    /**
     * Pipeline to converts equirectangular projection texture into a cubemap.
     */
    class IBLPipeline : public ComputePipeline {
    public:
        explicit IBLPipeline(Device &device);
        ~IBLPipeline() override = default;

        void convert(const shared_ptr<Image>&   hdrFile,
                     const shared_ptr<Cubemap>& cubemap) const;
        void preComputeSpecular(const shared_ptr<Cubemap>& unfilteredCubemap, const shared_ptr<Cubemap>& cubemap) const;
    private:
        struct SpecularFilterPushConstants {
            alignas(4) uint32_t level;
            alignas(4) float roughness;
        };

        unique_ptr<DescriptorPool> descriptorPool;
        unique_ptr<DescriptorSetLayout> descriptorSetLayout;

        static constexpr auto BINDING_INPUT_TEXTURE{0};
        static constexpr auto BINDING_OUTPUT_CUBEMAP{1};
        static constexpr auto BINDING_OUTPUT_CUBEMAP_MIPS{2};

        static constexpr auto specializationMap = VkSpecializationMapEntry { 0, 0, sizeof(uint32_t) };
        static constexpr uint32_t specializationData[]{ EnvironmentCubemap::ENVIRONMENT_MAP_MIPMAP_LEVELS - 1 };
        static constexpr auto specializationInfo = VkSpecializationInfo{ 1, &specializationMap, sizeof(specializationData), specializationData };
    };

}
