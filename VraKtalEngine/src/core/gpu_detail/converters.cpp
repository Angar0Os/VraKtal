#include "../src/core/gpu_detail/converters.h"

vk::ImageLayout core::gpu_detail::ToVulkan(ImageLayout layout)
{
    switch (layout)
    {
        case ImageLayout::ShaderReadOnly: return vk::ImageLayout::eShaderReadOnlyOptimal;
        case ImageLayout::ColorAttachment: return vk::ImageLayout::eColorAttachmentOptimal;
        case ImageLayout::DepthStencilAttachment: return vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case ImageLayout::TransferSrc: return vk::ImageLayout::eTransferSrcOptimal;
        case ImageLayout::TransferDst: return vk::ImageLayout::eTransferDstOptimal;
        case ImageLayout::Present: return vk::ImageLayout::ePresentSrcKHR;
        default: return vk::ImageLayout::eUndefined;
    }
}

vk::Filter core::gpu_detail::ToVulkan(Filter filter)
{
    switch (filter)
    {
        case Filter::Nearest: return vk::Filter::eNearest;
        case Filter::Linear: return vk::Filter::eLinear;
        default: return vk::Filter::eNearest;
    }
}