#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/commandBuffer_impl.h"
#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu_detail/converters.h"

#include <stdexcept>

core::gpu::Image::Impl::Impl(core::gpu::Image& p, vk::raii::Device& dev,
    vk::raii::PhysicalDevice& physDev, const ImageCreateInfo& info)
    : parent(p), device(dev), physicalDevice(physDev),
    image(nullptr), memory(nullptr), view(nullptr),
    width(info.width), height(info.height),
    mipLevels(info.mipLevels), arrayLayers(info.arrayLayers),
    format(info.format), samples(info.samples)
{
    if (info.width == 0 || info.height == 0)
    {
        throw std::runtime_error("Image width and height cannot be zero");
    }

    vk::ImageCreateInfo imageInfo{};
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = core::gpu_detail::ToVulkan(info.format);
    imageInfo.extent.width = info.width;
    imageInfo.extent.height = info.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = info.mipLevels;
    imageInfo.arrayLayers = info.arrayLayers;
    imageInfo.samples = core::gpu_detail::ToVulkan(info.samples);
    imageInfo.tiling = core::gpu_detail::ToVulkan(info.tiling);
    imageInfo.usage = core::gpu_detail::ToVulkan(info.usage);
    imageInfo.sharingMode = vk::SharingMode::eExclusive;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    image = vk::raii::Image(device, imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        core::gpu_detail::ToVulkan(info.memoryProperties)
    );

    memory = vk::raii::DeviceMemory(device, allocInfo);

    image.bindMemory(*memory, 0);
}

core::gpu::Image::Impl::~Impl() = default;

uint32_t core::gpu::Image::Impl::FindMemoryType(uint32_t typeFilter,
    vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type for image");
}

void core::gpu::Image::Impl::CreateView(const ImageViewCreateInfo& info)
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.image = *image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = core::gpu_detail::ToVulkan(info.format);
    viewInfo.subresourceRange.aspectMask = info.isDepth ?
        vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    viewInfo.subresourceRange.baseMipLevel = info.baseMipLevel;
    viewInfo.subresourceRange.levelCount = info.levelCount;
    viewInfo.subresourceRange.baseArrayLayer = info.baseArrayLayer;
    viewInfo.subresourceRange.layerCount = info.layerCount;

    view = vk::raii::ImageView(device, viewInfo);
}

void core::gpu::Image::Impl::TransitionLayout(CommandBuffer& commandBuffer,
    ImageLayout oldLayout, ImageLayout newLayout, uint32_t mipLevels)
{
    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout = core::gpu_detail::ToVulkan(oldLayout);
    barrier.newLayout = core::gpu_detail::ToVulkan(newLayout);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = *image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == ImageLayout::TransferDst && newLayout == ImageLayout::ShaderReadOnly)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (barrier.oldLayout == vk::ImageLayout::eUndefined &&
        barrier.newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition");
    }

    auto& cmdBuf = commandBuffer.GetImpl().GetCommandBuffer();
    cmdBuf.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);
}

void core::gpu::Image::Impl::CopyFromBuffer(CommandBuffer& commandBuffer,
    Buffer& buffer, uint32_t width, uint32_t height)
{
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = vk::Offset3D{ 0, 0, 0 };
    region.imageExtent = vk::Extent3D{ width, height, 1 };

    auto& cmdBuf = commandBuffer.GetImpl().GetCommandBuffer();
    auto& srcBuffer = buffer.GetImpl().GetBuffer();

    cmdBuf.copyBufferToImage(*srcBuffer, *image, vk::ImageLayout::eTransferDstOptimal, region);
}

void core::gpu::Image::Impl::GenerateMipmaps(CommandBuffer& commandBuffer,
    uint32_t width, uint32_t height, uint32_t mipLevels)
{
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(
        core::gpu_detail::ToVulkan(format));

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        throw std::runtime_error("Texture image format does not support linear blitting");
    }

    auto& cmdBuf = commandBuffer.GetImpl().GetCommandBuffer();

    vk::ImageMemoryBarrier barrier{};
    barrier.image = *image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for (uint32_t i = 1; i < mipLevels; i++)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        cmdBuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {}, nullptr, nullptr, barrier
        );

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.dstOffsets[1] = vk::Offset3D{
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
        };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        cmdBuf.blitImage(
            *image, vk::ImageLayout::eTransferSrcOptimal,
            *image, vk::ImageLayout::eTransferDstOptimal,
            blit, vk::Filter::eLinear
        );

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        cmdBuf.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {}, nullptr, nullptr, barrier
        );

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    cmdBuf.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {}, nullptr, nullptr, barrier
    );
}

vk::raii::Image& core::gpu::Image::Impl::GetImage()
{
    return image;
}

const vk::raii::Image& core::gpu::Image::Impl::GetImage() const
{
    return image;
}

vk::raii::ImageView& core::gpu::Image::Impl::GetView()
{
    return view;
}

const vk::raii::ImageView& core::gpu::Image::Impl::GetView() const
{
    return view;
}

core::gpu::Image::Image(void* device, void* physicalDevice, const ImageCreateInfo& info)
{
    auto& vkDevice = *static_cast<vk::raii::Device*>(device);
    auto& vkPhysicalDevice = *static_cast<vk::raii::PhysicalDevice*>(physicalDevice);

    m_impl = std::make_unique<Impl>(*this, vkDevice, vkPhysicalDevice, info);
}

core::gpu::Image::~Image() = default;

void* core::gpu::Image::GetHandle() const
{
    return static_cast<void*>(const_cast<vk::Image*>(&(*m_impl->GetImage())));
}

void* core::gpu::Image::GetViewHandle() const
{
    return static_cast<void*>(const_cast<vk::ImageView*>(&(*m_impl->GetView())));
}

uint32_t core::gpu::Image::GetWidth() const
{
    return m_impl->GetWidth();
}

uint32_t core::gpu::Image::GetHeight() const
{
    return m_impl->GetHeight();
}

uint32_t core::gpu::Image::GetMipLevels() const
{
    return m_impl->GetMipLevels();
}

uint32_t core::gpu::Image::GetArrayLayers() const
{
    return m_impl->GetArrayLayers();
}

core::TextureFormat core::gpu::Image::GetFormat() const
{
    return m_impl->GetFormat();
}

void core::gpu::Image::CreateView(const ImageViewCreateInfo& info)
{
    m_impl->CreateView(info);
}

void core::gpu::Image::TransitionLayout(CommandBuffer& commandBuffer,
    ImageLayout oldLayout, ImageLayout newLayout, uint32_t mipLevels)
{
    m_impl->TransitionLayout(commandBuffer, oldLayout, newLayout, mipLevels);
}

void core::gpu::Image::CopyFromBuffer(CommandBuffer& commandBuffer,
    Buffer& buffer, uint32_t width, uint32_t height)
{
    m_impl->CopyFromBuffer(commandBuffer, buffer, width, height);
}

void core::gpu::Image::GenerateMipmaps(CommandBuffer& commandBuffer,
    uint32_t width, uint32_t height, uint32_t mipLevels)
{
    m_impl->GenerateMipmaps(commandBuffer, width, height, mipLevels);
}

core::gpu::Image::Impl& core::gpu::Image::GetImpl()
{
    return *m_impl;
}