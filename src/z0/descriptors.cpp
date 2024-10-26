/*
* Derived from
 * https://github.com/blurrypiano/littleVulkanEngine
 * and
 * https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer
*/
module;
#include <cstdlib>
#include "z0/libraries.h"
#include <cassert>
#include <volk.h>

module z0;

import :Tools;
import :Device;
import :Descriptors;

namespace z0 {
    DescriptorSetLayout::Builder::Builder(const Device &dev) :
        device{dev} {
    }

    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(const uint32_t           binding,
                                                                           const VkDescriptorType   descriptorType,
                                                                           const VkShaderStageFlags stageFlags,
                                                                           const uint32_t           count,
                                                                           const VkSampler*         immutableSampler) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        const VkDescriptorSetLayoutBinding layoutBinding{
                .binding = binding,
                .descriptorType = descriptorType,
                .descriptorCount = count == 0 ? 1 : count,
                .stageFlags = stageFlags,
                .pImmutableSamplers = immutableSampler
        };
        bindings[binding] = layoutBinding;
        return *this;
    }

    unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return make_unique<DescriptorSetLayout>(device, bindings);
    }

    DescriptorSetLayout::DescriptorSetLayout(const Device &device,
                                             const unordered_map<uint32_t,
                                                                 VkDescriptorSetLayoutBinding> &bindings):
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

    DescriptorPool::Builder::Builder(const Device &dev) :
        device{dev} {
    }

    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(const VkDescriptorType descriptorType,
                                                                  const uint32_t         count) {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(const VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(const uint32_t count) {
        maxSets = count;
        return *this;
    }

    unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
    }

    DescriptorPool::DescriptorPool(
            const Device &                      device,
            const uint32_t                      maxSets,
            const VkDescriptorPoolCreateFlags   poolFlags,
            const vector<VkDescriptorPoolSize> &poolSizes):
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


    bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout &descriptorSetLayout,
                                            VkDescriptorSet &            descriptor) const {
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
        return true;
    }

    void DescriptorPool::freeDescriptors(const vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(
                device.getDevice(),
                descriptorPool,
                static_cast<uint32_t>(descriptors.size()),
                descriptors.data());
    }

    void DescriptorPool::resetPool() const {
        vkResetDescriptorPool(device.getDevice(), descriptorPool, 0);
    }


    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool):
        setLayout{setLayout},
        pool{pool} {
    }

    DescriptorWriter &DescriptorWriter::writeBuffer(const uint32_t binding, const VkDescriptorBufferInfo *bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        const auto &               bindingDescription = setLayout.bindings[binding];
        const VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = binding,
                .descriptorCount = bindingDescription.descriptorCount,
                .descriptorType = bindingDescription.descriptorType,
                .pBufferInfo = bufferInfo,
        };
        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(const uint32_t binding, const VkDescriptorImageInfo *imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        const auto &               bindingDescription = setLayout.bindings[binding];
        const VkWriteDescriptorSet write{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = binding,
                .descriptorCount = bindingDescription.descriptorCount,
                .descriptorType = bindingDescription.descriptorType,
                .pImageInfo = imageInfo,
        };
        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet &set, const bool create) {
        if (create && (!pool.allocateDescriptor(*setLayout.getDescriptorSetLayout(), set)))
            return false;
        update(set);
        return true;
    }

    void DescriptorWriter::update(const VkDescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.getDevice(),
            writes.size(), writes.data(),
            0, nullptr);
    }


}
