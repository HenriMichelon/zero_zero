/*
* Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
*/
module;
#include "z0/modules.h"

export module Z0:Descriptors;

import :Tools;
import :Device;

export namespace z0 {

    /**
     * Vulkan [VkDescriptorSetLayout](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayout.html) helper
     */
    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(const Device &dev) : device{dev} {}

            Builder& addBinding(uint32_t binding,
                                VkDescriptorType descriptorType,
                                VkShaderStageFlags stageFlags,
                                uint32_t count = 1);
            unique_ptr<DescriptorSetLayout> build() const;

        private:
            const Device &device;
            unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(const Device &Device, const unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        [[nodiscard]] VkDescriptorSetLayout* getDescriptorSetLayout() { return &descriptorSetLayout; }

    private:
        const Device &device;
        VkDescriptorSetLayout descriptorSetLayout;
        unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    /**
     * Vulkan [VkDescriptorPool](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorPool.html) helper
     */
    class DescriptorPool {
    public:
        class Builder {
        public:
            explicit Builder(const Device &dev) : device{dev} {}

            [[nodiscard]] Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            [[nodiscard]] Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            [[nodiscard]] Builder &setMaxSets(uint32_t count);
            [[nodiscard]] unique_ptr<DescriptorPool> build() const;

        private:
            const Device &device;
            vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        DescriptorPool(
                const Device &device,
                uint32_t maxSets,
                VkDescriptorPoolCreateFlags poolFlags,
                const vector<VkDescriptorPoolSize> &poolSizes);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet &descriptor) const;
        void freeDescriptors(vector<VkDescriptorSet> &descriptors) const;
        void resetPool();
        VkDescriptorPool getPool() const { return descriptorPool; }

    private:
        const Device &device;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;
    };

    /**
     * Vulkan {VkWriteDescriptorSet](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSet.html) helper
     */
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

        [[nodiscard]] DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        [[nodiscard]] DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        [[nodiscard]] bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        vector<VkWriteDescriptorSet> writes;
    };


    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{
            .binding = binding,
            .descriptorType = descriptorType,
            .descriptorCount = count == 0 ? 1 : count,
            .stageFlags = stageFlags,
        };
        bindings[binding] = layoutBinding;
        return *this;
    }

    unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return make_unique<DescriptorSetLayout>(device, bindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(const Device &device,
                                             const unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings):
            device{device},
            bindings{bindings} {
        vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }
        const VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
                .pBindings = setLayoutBindings.data(),
        };
        if (vkCreateDescriptorSetLayout(
                device.getDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &descriptorSetLayout) != VK_SUCCESS) {
            die("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
    }

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(
            VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(
            VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    DescriptorPool::DescriptorPool(const Device &device,
                                   uint32_t maxSets,
                                   VkDescriptorPoolCreateFlags poolFlags,
                                   const std::vector<VkDescriptorPoolSize> &poolSizes):
            device{device} {
        const VkDescriptorPoolCreateInfo descriptorPoolInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = poolFlags,
                .maxSets = maxSets,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data(),
        };
        if (vkCreateDescriptorPool(device.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            die("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout& descriptorSetLayout,
                                            VkDescriptorSet &descriptor) const {
        const VkDescriptorSetAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptorSetLayout,
        };
        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(device.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
#ifdef VULKAN_STATS
        VulkanStats::get().descriptorSetsCount += 1;
#endif
        return true;
    }

    void DescriptorPool::freeDescriptors(vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(
                device.getDevice(),
                descriptorPool,
                static_cast<uint32_t>(descriptors.size()),
                descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(device.getDevice(), descriptorPool, 0);
    }

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool):
        setLayout{setLayout},
        pool{pool} {}

    DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        const auto &bindingDescription = setLayout.bindings[binding];
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = bindingDescription.descriptorCount,
            .descriptorType = bindingDescription.descriptorType,
            .pBufferInfo = bufferInfo,
        };
        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        const auto &bindingDescription = setLayout.bindings[binding];
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = bindingDescription.descriptorCount,
            .descriptorType = bindingDescription.descriptorType,
            .pImageInfo = imageInfo,
        };
        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet &set) {
        auto success = pool.allocateDescriptor(*setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(VkDescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.getDevice(), writes.size(), writes.data(), 0, nullptr);
    }

}