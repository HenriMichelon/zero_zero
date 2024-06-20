#pragma once

namespace z0 {

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

        VkDescriptorSetLayout* getDescriptorSetLayout() { return &descriptorSetLayout; }

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

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            unique_ptr<DescriptorPool> build() const;

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