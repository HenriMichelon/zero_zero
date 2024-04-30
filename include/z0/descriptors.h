#pragma once

#include "z0/device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace z0 {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            explicit Builder(Device &dev) : device{dev} {}

            Builder& addBinding(
                    uint32_t binding,
                    VkDescriptorType descriptorType,
                    VkShaderStageFlags stageFlags,
                    uint32_t count = 1);
            unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &device;
            unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(
                Device &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        VkDescriptorSetLayout* getDescriptorSetLayout() { return &descriptorSetLayout; }

    private:
        Device &device;
        VkDescriptorSetLayout descriptorSetLayout;
        unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            explicit Builder(Device &dev) : device{dev} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            unique_ptr<DescriptorPool> build() const;

        private:
            Device &device;
            vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        DescriptorPool(
                Device &device,
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
        Device &device;
        VkDescriptorPool descriptorPool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &setLayout, DescriptorPool &pool);

        DescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        DescriptorSetLayout &setLayout;
        DescriptorPool &pool;
        vector<VkWriteDescriptorSet> writes;
    };

}