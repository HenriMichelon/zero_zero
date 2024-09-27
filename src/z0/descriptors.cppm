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
            explicit Builder(const Device &dev) :
                device{dev} {
            }

            Builder &addBinding(const uint32_t           binding,
                                const VkDescriptorType   descriptorType,
                                const VkShaderStageFlags stageFlags,
                                const uint32_t           count = 1) {
                assert(bindings.count(binding) == 0 && "Binding already in use");
                const VkDescriptorSetLayoutBinding layoutBinding{
                        .binding = binding,
                        .descriptorType = descriptorType,
                        .descriptorCount = count == 0 ? 1 : count,
                        .stageFlags = stageFlags,
                };
                bindings[binding] = layoutBinding;
                return *this;
            }

            unique_ptr<DescriptorSetLayout> build() const {
                return make_unique<DescriptorSetLayout>(device, bindings);
            }

        private:
            const Device &                                        device;
            unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(const Device &device,
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

        ~DescriptorSetLayout() {
            vkDestroyDescriptorSetLayout(device.getDevice(), descriptorSetLayout, nullptr);
        }

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;

        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        [[nodiscard]] VkDescriptorSetLayout *getDescriptorSetLayout() { return &descriptorSetLayout; }

    private:
        const Device &                                        device;
        VkDescriptorSetLayout                                 descriptorSetLayout;
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
            explicit Builder(const Device &dev) :
                device{dev} {
            }

            [[nodiscard]] Builder &addPoolSize(const VkDescriptorType descriptorType, const uint32_t count) {
                poolSizes.push_back({descriptorType, count});
                return *this;
            }

            [[nodiscard]] Builder &setPoolFlags(const VkDescriptorPoolCreateFlags flags) {
                poolFlags = flags;
                return *this;
            }

            [[nodiscard]] Builder &setMaxSets(const uint32_t count) {
                maxSets = count;
                return *this;
            }

            [[nodiscard]] unique_ptr<DescriptorPool> build() const {
                return std::make_unique<DescriptorPool>(device, maxSets, poolFlags, poolSizes);
            }

        private:
            const Device &               device;
            vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t                     maxSets   = 1000;
            VkDescriptorPoolCreateFlags  poolFlags = 0;
        };

        DescriptorPool(
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

        ~DescriptorPool() {
            vkDestroyDescriptorPool(device.getDevice(), descriptorPool, nullptr);
        }

        DescriptorPool(const DescriptorPool &) = delete;

        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorSet &descriptor) const {
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

        void freeDescriptors(const vector<VkDescriptorSet> &descriptors) const {
            vkFreeDescriptorSets(
                    device.getDevice(),
                    descriptorPool,
                    static_cast<uint32_t>(descriptors.size()),
                    descriptors.data());
        }

        void resetPool() const {
            vkResetDescriptorPool(device.getDevice(), descriptorPool, 0);
        }

        VkDescriptorPool getPool() const { return descriptorPool; }

    private:
        const Device &   device;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;
    };

    /**
     * Vulkan [VkWriteDescriptorSet](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkWriteDescriptorSet.html) helper
     */
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool):
            setLayout{setLayout},
            pool{pool} {
        }

        [[nodiscard]] DescriptorWriter &writeBuffer(const uint32_t binding, const VkDescriptorBufferInfo *bufferInfo) {
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

        [[nodiscard]] DescriptorWriter &writeImage(const uint32_t binding, const VkDescriptorImageInfo *imageInfo) {
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

        [[nodiscard]] bool build(VkDescriptorSet &set) {
            const auto success = pool.allocateDescriptor(*setLayout.getDescriptorSetLayout(), set);
            if (!success) {
                return false;
            }
            overwrite(set);
            return true;
        }

        void overwrite(const VkDescriptorSet &set) {
            for (auto &write : writes) {
                write.dstSet = set;
            }
            vkUpdateDescriptorSets(pool.device.getDevice(), writes.size(), writes.data(), 0, nullptr);
        }

    private:
        DescriptorSetLayout &        setLayout;
        DescriptorPool &             pool;
        vector<VkWriteDescriptorSet> writes;
    };
}
