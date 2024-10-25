module;
#include <volk.h>
#include "z0/libraries.h"

export module z0:Equirect2CubemapPipeline;

import :Device;
import :ComputePipeline;
import :Descriptors;
import :Image;
import :Cubemap;

export namespace z0 {

    /**
     * Pipeline to converts equirectangular projection texture into a cubemap.
     */
    class Equirect2CubemapPipeline : public ComputePipeline {
    public:
        Equirect2CubemapPipeline(Device &device);
        ~Equirect2CubemapPipeline() override = default;

        void convert(const shared_ptr<Image>&   hdrFile,
                     const shared_ptr<Cubemap>& cubemap) const;
    private:
        unique_ptr<DescriptorPool> descriptorPool;
        unique_ptr<DescriptorSetLayout> descriptorSetLayout;
    };

}
