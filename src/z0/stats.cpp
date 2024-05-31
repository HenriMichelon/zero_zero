#include "z0/stats.h"

namespace z0 {

#ifdef VULKAN_STATS
    unique_ptr<VulkanStats> VulkanStats::instance = make_unique<VulkanStats>();

    void VulkanStats::display() const {
        cout << buffersCount << " buffers" << endl;
        cout << descriptorSetsCount << " descriptor sets" << endl;
        cout << imagesCount << " images" << endl;
        cout << averageFps << " avg FPS" << endl;
    }
#endif

}