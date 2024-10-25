module;
#include "z0/libraries.h"
#include <volk.h>

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

        void convert(const shared_ptr<Image>&   hdrFile,
                     const shared_ptr<Cubemap>& cubemap) const;
    private:
        Device &device;

        vector<char> readFile(const string &fileName) const;
    };

}
