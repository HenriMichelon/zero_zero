/*
 * Copyright (c) 2024-2025 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include "z0/libraries.h"
#include "z0/vulkan.h"

export module z0.vulkan.Descriptors;

import z0.vulkan.Device;

export namespace z0 {

    /*
     * Vulkan [VkDescriptorSetLayout](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkDescriptorSetLayout.html) helper
     */
    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(const Device &dev);

            Builder &addBinding(uint32_t           binding,
                                VkDescriptorType   descriptorType,
                                VkShaderStageFlags stageFlags,
                                uint32_t           count = 1,
                                const VkSampler*   immutableSampler = VK_NULL_HANDLE);

            unique_ptr<DescriptorSetLayout> build() const;

        private:
            const Device &                                        device;
            unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(const Device &device,
                            const unordered_map<uint32_t,
                                                VkDescriptorSetLayoutBinding> &bindings);

        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;

        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        [[nodiscard]] inline VkDescriptorSetLayout *getDescriptorSetLayout() { return &descriptorSetLayout; }

        [[nodiscard]] inline bool isValid() const { return descriptorSetLayout != VK_NULL_HANDLE; }

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
            explicit Builder(const Device &dev);

            [[nodiscard]] Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);

            [[nodiscard]] Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);

            [[nodiscard]] Builder &setMaxSets(uint32_t count);

            [[nodiscard]] unique_ptr<DescriptorPool> build() const;

        private:
            const Device &               device;
            vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t                     maxSets   = 1000;
            VkDescriptorPoolCreateFlags  poolFlags = 0;
        };

        DescriptorPool(
                const Device &                      device,
                uint32_t                            maxSets,
                VkDescriptorPoolCreateFlags         poolFlags,
                const vector<VkDescriptorPoolSize> &poolSizes);

        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;

        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout &descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(const vector<VkDescriptorSet> &descriptors) const;

        void resetPool() const;

        VkDescriptorPool inline getPool() const { return descriptorPool; }

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
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

        DescriptorWriter &writeBuffer(uint32_t binding, const VkDescriptorBufferInfo *bufferInfo);

        DescriptorWriter &writeImage(uint32_t binding, const VkDescriptorImageInfo *imageInfo);

        [[nodiscard]] bool build(VkDescriptorSet &set, bool create);

        void update(const VkDescriptorSet &set);

    private:
        DescriptorSetLayout &        setLayout;
        DescriptorPool &             pool;
        vector<VkWriteDescriptorSet> writes;
    };
}
