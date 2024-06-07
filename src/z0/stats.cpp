#include "z0/stats.h"

namespace z0 {

#ifdef VULKAN_STATS
    unique_ptr<VulkanStats> VulkanStats::instance = make_unique<VulkanStats>();

    void VulkanStats::display() const {
        log(to_string(buffersCount), " buffers");
        log(to_string(descriptorSetsCount), " descriptor sets");
        log(to_string(imagesCount), " images");
        log(to_string(averageFps), " avg FPS");
    }
#endif

}