#pragma once

#include <cstdint>
#include <memory>
using namespace std;

namespace z0 {

#ifdef VULKAN_STATS
    class VulkanStats {
    public:
        uint32_t buffersCount{0};
        uint32_t descriptorSetsCount{0};
        uint32_t imagesCount{0};
        uint32_t averageFps{0};

        void display() const;

        static VulkanStats& get() { return *instance; }
    private:
        static unique_ptr<VulkanStats> instance;
    };
#endif

}