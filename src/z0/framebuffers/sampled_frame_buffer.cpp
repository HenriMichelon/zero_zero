#include "z0/z0.h"

namespace z0 {

       VkDescriptorImageInfo SampledFrameBuffer::imageInfo() {
        return VkDescriptorImageInfo {
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

}