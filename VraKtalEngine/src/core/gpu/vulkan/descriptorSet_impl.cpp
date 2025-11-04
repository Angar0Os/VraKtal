#include "../src/core/gpu/vulkan/descriptorSet_impl.h"

core::gpu::DescriptorSet::Impl::Impl(vk::raii::Device& dev, std::vector<vk::raii::DescriptorSet>& sets, size_t frame)
	: device(dev), descriptorSets(sets), currentFrame(frame)
{

}

//core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindBuffer(core::BufferHandle buffer, size_t offset, size_t range)
//{
//
//}
//
//core::gpu::DescriptorSet& core::gpu::DescriptorSet::Impl::BindImage(core::SamplerHandle samplerHandle, const core::TextureSet* texture, const core::TextureSet& defaultTexture, core::ImageLayout layout)
//{
//
//}

//
//struct TextureSet
//{
//    vk::raii::Image image = nullptr;
//    vk::raii::DeviceMemory memory = nullptr;
//    vk::raii::ImageView view = nullptr;
//    uint32_t mipLevels = 1;
//
//    bool isValid() const { return view != nullptr; }
//};
//
//class DescriptorSetBuilder
//{
//private:
//    vk::raii::Device& device;
//    std::vector<vk::raii::DescriptorSet>& descriptorSets;
//    size_t currentFrame;
//    uint32_t currentBinding = 0;
//    std::vector<vk::DescriptorBufferInfo> bufferInfos;
//    std::vector<vk::DescriptorImageInfo> imageInfos;
//    std::vector<vk::WriteDescriptorSet> writes;
//
//public:
//    DescriptorSetBuilder(vk::raii::Device& dev,
//        std::vector<vk::raii::DescriptorSet>& sets,
//        size_t frame)
//        : device(dev), descriptorSets(sets), currentFrame(frame)
//    {
//    }
//
//    DescriptorSetBuilder& BindBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize range)
//    {
//        bufferInfos.push_back(vk::DescriptorBufferInfo{
//            .buffer = buffer,
//            .offset = offset,
//            .range = range
//            });
//
//        writes.push_back(vk::WriteDescriptorSet{
//            .dstSet = descriptorSets[currentFrame],
//            .dstBinding = currentBinding++,
//            .dstArrayElement = 0,
//            .descriptorCount = 1,
//            .descriptorType = vk::DescriptorType::eUniformBuffer,
//            .pBufferInfo = &bufferInfos.back()
//            });
//
//        return *this;
//    }
//
//    DescriptorSetBuilder& BindImage(vk::Sampler sampler,
//        const TextureSet* texture,
//        const TextureSet& defaultTexture,
//        vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal)
//    {
//        imageInfos.push_back(vk::DescriptorImageInfo{
//            .sampler = sampler,
//            .imageView = (texture && texture->isValid()) ? *texture->view : *defaultTexture.view,
//            .imageLayout = layout
//            });
//
//        writes.push_back(vk::WriteDescriptorSet{
//            .dstSet = descriptorSets[currentFrame],
//            .dstBinding = currentBinding++,
//            .dstArrayElement = 0,
//            .descriptorCount = 1,
//            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
//            .pImageInfo = &imageInfos.back()
//            });
//
//        return *this;
//    }
//
//    void Update()
//    {
//        if (!writes.empty())
//        {
//            device.updateDescriptorSets(writes, {});
//        }
//        currentBinding = 0;
//    }
//};
//
//void CreateDescriptorSets()
//{
//    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
//    vk::DescriptorSetAllocateInfo allocInfo{
//        .descriptorPool = descriptorPool,
//        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
//        .pSetLayouts = layouts.data()
//    };
//
//    descriptorSets.clear();
//    descriptorSets = device.allocateDescriptorSets(allocInfo);
//
//    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
//    {
//        DescriptorSetBuilder(device, descriptorSets, i)
//            .BindBuffer(uniformBuffers[i], 0, sizeof(UniformBufferObject))
//            .BindImage(textureSampler, albedoTexture, defaultWhiteTexture)
//            .BindImage(textureSampler, normalTexture, defaultNormalTexture)
//            .BindImage(textureSampler, metallicTexture, defaultWhiteTexture)
//            .BindImage(textureSampler, roughnessTexture, defaultWhiteTexture)
//            .BindImage(textureSampler, aoTexture, defaultWhiteTexture)
//            .BindImage(textureSampler, emissiveTexture, defaultBlackTexture)
//            .BindImage(shadowSampler, &shadowMapTexture, defaultWhiteTexture)
//            .Update();
//    }
//}