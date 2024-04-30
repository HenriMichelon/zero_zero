/*
 * Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
*/
#include "z0/tools.h"
#include "z0/descriptors.h"
#include "z0/stats.h"

#include <cassert>
#include <stdexcept>

namespace z0 {

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

    DescriptorSetLayout::DescriptorSetLayout(
            Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings):
            device{device},
            bindings{bindings} {
        vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
        if (vkCreateDescriptorSetLayout(
                device.getVkDevice(),
                &descriptorSetLayoutInfo,
                nullptr,
                &descriptorSetLayout) != VK_SUCCESS) {
            die("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getVkDevice(), descriptorSetLayout, nullptr);
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

    DescriptorPool::DescriptorPool(
            Device &device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes):
            device{device} {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(device.getVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            die("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.getVkDevice(), descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptor(
            const VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet &descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(device.getVkDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
#ifdef VULKAN_STATS
        VulkanStats::get().descriptorSetsCount += 1;
#endif
        return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(
                device.getVkDevice(),
                descriptorPool,
                static_cast<uint32_t>(descriptors.size()),
                descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(device.getVkDevice(), descriptorPool, 0);
    }

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool):
        setLayout{setLayout},
        pool{pool} {}

    DescriptorWriter &DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
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
        auto &bindingDescription = setLayout.bindings[binding];
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
        bool success = pool.allocateDescriptor(*setLayout.getDescriptorSetLayout(), set);
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
        vkUpdateDescriptorSets(pool.device.getVkDevice(), writes.size(), writes.data(), 0, nullptr);
    }

}
