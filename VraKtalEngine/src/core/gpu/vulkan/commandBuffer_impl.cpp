#include "../src/core/gpu/vulkan/accelerationStructure_impl.h"
#include "../src/core/gpu/vulkan/buffer_impl.h"
#include "../src/core/gpu/vulkan/commandBuffer_impl.h"
#include "../src/core/gpu/vulkan/commandPool_impl.h"
#include "../src/core/gpu/vulkan/device_impl.h"
#include "../src/core/gpu/vulkan/descriptorSet_impl.h"
#include "../src/core/gpu/vulkan/image_impl.h"
#include "../src/core/gpu/vulkan/pipeline_impl.h"

#include <stdexcept>

using namespace core::gpu;

core::gpu::CommandBuffer::Impl::Impl(core::gpu::CommandBuffer& p, const core::gpu::Device* device, const SCommandBufferCreateInfo& info)
	: parent(p), commandBuffers(nullptr), isSingleTime(info.singleTime), currentIndex(0)
{
	if (info.count == 0)
	{
		throw std::runtime_error("CommandBuffer count cannot be zero");
	}

	vk::CommandBufferLevel level = (info.level == ECommandBufferLevel::Primary)
		? vk::CommandBufferLevel::ePrimary
		: vk::CommandBufferLevel::eSecondary;

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = device->GetImpl().commandPool->GetImpl().pool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = info.count;

	commandBuffers = vk::raii::CommandBuffers(device->GetImpl().device, allocInfo);
}

core::gpu::CommandBuffer::Impl::~Impl()
{
}

vk::raii::CommandBuffer& core::gpu::CommandBuffer::Impl::GetCommandBuffer(uint32_t index)
{
	if (index >= commandBuffers.size())
	{
		throw std::out_of_range("CommandBuffer index out of range");
	}
	return commandBuffers[index];
}

const vk::raii::CommandBuffer& core::gpu::CommandBuffer::Impl::GetCommandBuffer(uint32_t index) const
{
	if (index >= commandBuffers.size())
	{
		throw std::out_of_range("CommandBuffer index out of range");
	}
	return commandBuffers[index];
}

void core::gpu::CommandBuffer::Impl::BuildAccelerationStructure(const core::gpu::AccelerationStructure* accelerationStructure)
{
	accelerationStructure->GetImpl().Build(GetCommandBuffer(currentIndex));
}

void core::gpu::CommandBuffer::Impl::AccelerationStructureBarrier()
{
	vk::MemoryBarrier barrier{};
	barrier.srcAccessMask = vk::AccessFlagBits::eAccelerationStructureWriteKHR;
	barrier.dstAccessMask = vk::AccessFlagBits::eAccelerationStructureReadKHR;

	GetCommandBuffer(currentIndex).pipelineBarrier(
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		{},
		{ barrier },
		{},
		{}
	);
}

void core::gpu::CommandBuffer::Impl::BindRayTracingPipeline(const core::gpu::Pipeline* pipeline)
{
	GetCommandBuffer(currentIndex).bindPipeline(
		vk::PipelineBindPoint::eRayTracingKHR,
		vk::Pipeline(pipeline->GetImpl().pipeline)
	);
}

void core::gpu::CommandBuffer::Impl::TraceRays(
	const core::gpu::Device* device,
	void* raygenSBT, uint32_t raygenOffset, uint32_t raygenStride,
	void* missSBT, uint32_t missOffset, uint32_t missStride, uint32_t missCount,
	void* hitSBT, uint32_t hitOffset, uint32_t hitStride, uint32_t hitCount,
	void* callableSBT, uint32_t callableOffset, uint32_t callableStride, uint32_t callableCount,
	uint32_t width, uint32_t height, uint32_t depth)
{
	vk::StridedDeviceAddressRegionKHR raygenRegion{};
	if (raygenSBT)
	{
		vk::Buffer raygenBuffer = static_cast<VkBuffer>(raygenSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = raygenBuffer;

		raygenRegion.deviceAddress = device->GetImpl().device.getBufferAddress(addressInfo) + raygenOffset;
		raygenRegion.stride = raygenStride;
		raygenRegion.size = raygenStride;
	}

	vk::StridedDeviceAddressRegionKHR missRegion{};
	if (missSBT && missCount > 0)
	{
		vk::Buffer missBuffer = static_cast<VkBuffer>(missSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = missBuffer;

		missRegion.deviceAddress = device->GetImpl().device.getBufferAddress(addressInfo) + missOffset;
		missRegion.stride = missStride;
		missRegion.size = missStride * missCount;
	}

	vk::StridedDeviceAddressRegionKHR hitRegion{};
	if (hitSBT && hitCount > 0)
	{
		vk::Buffer hitBuffer = static_cast<VkBuffer>(hitSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = hitBuffer;

		hitRegion.deviceAddress = device->GetImpl().device.getBufferAddress(addressInfo) + hitOffset;
		hitRegion.stride = hitStride;
		hitRegion.size = hitStride * hitCount;
	}

	vk::StridedDeviceAddressRegionKHR callableRegion{};
	if (callableSBT && callableCount > 0)
	{
		vk::Buffer callableBuffer = static_cast<VkBuffer>(callableSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = callableBuffer;

		callableRegion.deviceAddress = device->GetImpl().device.getBufferAddress(addressInfo) + callableOffset;
		callableRegion.stride = callableStride;
		callableRegion.size = callableStride * callableCount;
	}

	GetCommandBuffer(currentIndex).traceRaysKHR(
		raygenRegion,
		missRegion,
		hitRegion,
		callableRegion,
		width,
		height,
		depth
	);
}

uint32_t core::gpu::CommandBuffer::Impl::GetCount() const
{
	return static_cast<uint32_t>(commandBuffers.size());
}

bool core::gpu::CommandBuffer::Impl::IsSingleTime() const
{
	return isSingleTime;
}

void core::gpu::CommandBuffer::Impl::Begin(uint32_t index)
{
	if (index >= commandBuffers.size())
	{
		throw std::out_of_range("CommandBuffer index out of range");
	}

	currentIndex = index;

	vk::CommandBufferBeginInfo beginInfo{};

	if (isSingleTime)
	{
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	}

	commandBuffers[index].begin(beginInfo);
}

void core::gpu::CommandBuffer::Impl::End(uint32_t index)
{
	if (index >= commandBuffers.size())
	{
		throw std::out_of_range("CommandBuffer index out of range");
	}

	commandBuffers[index].end();
}

void core::gpu::CommandBuffer::Impl::PushConstants(const core::gpu::Pipeline* pipeline,
	uint32_t stageFlags,
	uint32_t offset,
	uint32_t size,
	const void* pValues)
{
	vk::ShaderStageFlags vkStageFlags;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Vertex))
		vkStageFlags |= vk::ShaderStageFlagBits::eVertex;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Fragment))
		vkStageFlags |= vk::ShaderStageFlagBits::eFragment;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Compute))
		vkStageFlags |= vk::ShaderStageFlagBits::eCompute;

	GetCommandBuffer(currentIndex).pushConstants<uint8_t>(
		pipeline->GetImpl().pipelineLayout,
		vkStageFlags,
		offset,
		vk::ArrayProxy<const uint8_t>(size, static_cast<const uint8_t*>(pValues))
	);
}

void core::gpu::CommandBuffer::Impl::BindVertexBuffer(const core::gpu::Buffer* buffer, size_t offset)
{
	vk::DeviceSize vkOffset = static_cast<vk::DeviceSize>(offset);
	GetCommandBuffer(currentIndex).bindVertexBuffers(0, *buffer->GetImpl().buffer, vkOffset);
}

void core::gpu::CommandBuffer::Impl::BindIndexBuffer(const core::gpu::Buffer* buffer, size_t offset)
{
	GetCommandBuffer(currentIndex).bindIndexBuffer(
		buffer->GetImpl().buffer,
		static_cast<vk::DeviceSize>(offset),
		vk::IndexType::eUint32
	);
}

void CommandBuffer::BindDescriptorSets(const Pipeline* currentPipeline, const DescriptorSet* descriptorSet, uint32_t frameIndex, uint32_t firstSet)
{
	m_impl->GetCommandBuffer(m_impl->currentIndex).bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		currentPipeline->GetImpl().pipelineLayout,
		firstSet,
		*descriptorSet->GetImpl().descriptorSet,
		nullptr
	);
}

void core::gpu::CommandBuffer::Impl::SetViewport(float x, float y, const core::gpu::Device* device, float minDepth, float maxDepth)
{
	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::Viewport viewport(x, y, static_cast<float>(width), static_cast<float>(height), minDepth, maxDepth);
	GetCommandBuffer(currentIndex).setViewport(0, viewport);
}

void core::gpu::CommandBuffer::Impl::SetScissor(int32_t x, int32_t y, const core::gpu::Device* device)
{
	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::Rect2D scissor({ x, y }, { width, height });
	GetCommandBuffer(currentIndex).setScissor(0, scissor);
}

void core::gpu::CommandBuffer::Impl::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
	uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	GetCommandBuffer(currentIndex).drawIndexed(
		indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void core::gpu::CommandBuffer::Impl::BeginRendering(
	const core::gpu::Device* device,
	const core::gpu::Image* colorImage,
	const core::gpu::Image* depthImage)
{
	vk::RenderingAttachmentInfo colorAttachment{};
	colorAttachment.imageView = colorImage->GetImpl().view;
	colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.f);

	vk::RenderingAttachmentInfo depthAttachment{};
	depthAttachment.imageView = depthImage->GetImpl().view;
	depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.clearValue = vk::ClearDepthStencilValue(1.f, 0);

	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::RenderingInfo info{};
	info.renderArea = vk::Rect2D({ 0, 0 }, { width, height });
	info.layerCount = 1;
	info.colorAttachmentCount = 1;
	info.pColorAttachments = &colorAttachment;
	info.pDepthAttachment = &depthAttachment;

	GetCommandBuffer(currentIndex).beginRendering(info);
}

void core::gpu::CommandBuffer::Impl::BeginRendering(
	const core::gpu::Device* device,
	const std::vector<CommandBuffer::RenderingAttachmentInfo>& colorAttachments,
	const CommandBuffer::DepthAttachmentInfo& depthAttachment)
{
	std::vector<vk::RenderingAttachmentInfo> vkColorAttachments;
	vkColorAttachments.reserve(colorAttachments.size());

	for (const auto& ca : colorAttachments)
	{
		vk::RenderingAttachmentInfo info{};
		info.imageView = ca.image->GetImpl().view;
		info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		info.loadOp = ca.clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
		info.storeOp = vk::AttachmentStoreOp::eStore;
		info.clearValue = vk::ClearColorValue(ca.clearR, ca.clearG, ca.clearB, ca.clearA);
		vkColorAttachments.push_back(info);
	}

	vk::RenderingAttachmentInfo vkDepth{};
	if (depthAttachment.image)
	{
		vkDepth.imageView = depthAttachment.image->GetImpl().view;
		vkDepth.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		vkDepth.loadOp = depthAttachment.clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad;
		vkDepth.storeOp = vk::AttachmentStoreOp::eStore;
		vkDepth.clearValue = vk::ClearDepthStencilValue(depthAttachment.clearDepth, 0);
	}

	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::RenderingInfo info{};
	info.renderArea = vk::Rect2D({ 0, 0 }, { width, height });
	info.layerCount = 1;
	info.colorAttachmentCount = static_cast<uint32_t>(vkColorAttachments.size());
	info.pColorAttachments = vkColorAttachments.data();
	info.pDepthAttachment = depthAttachment.image ? &vkDepth : nullptr;

	GetCommandBuffer(currentIndex).beginRendering(info);
}

void core::gpu::CommandBuffer::Submit(const core::gpu::Device* device, uint32_t frameIndex)
{
	if (m_impl->currentIndex >= m_impl->commandBuffers.size())
	{
		throw std::runtime_error("No command buffer has been begun");
	}

	if (frameIndex >= device->GetImpl().frameSyncObjects.size())
	{
		throw std::runtime_error("Frame index out of range");
	}

	vk::CommandBuffer cmdBuf = *m_impl->commandBuffers[m_impl->currentIndex];

	auto& frameSync = device->GetImpl().frameSyncObjects[frameIndex];

	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	vk::Semaphore waitSemaphore = *frameSync.imageAvailable;
	vk::Semaphore signalSemaphore = *frameSync.renderFinished;

	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	device->GetImpl().graphicsQueue.submit(submitInfo, *frameSync.inFlightFence);
}

void core::gpu::CommandBuffer::SubmitAndWait(const core::gpu::Device* device, uint32_t frameIndex)
{
	Submit(device, frameIndex);
	device->GetImpl().graphicsQueue.waitIdle();
}

void core::gpu::CommandBuffer::SubmitImmediate(const core::gpu::Device* device)
{
	if (m_impl->currentIndex >= m_impl->commandBuffers.size())
	{
		throw std::runtime_error("No command buffer has been begun");
	}

	vk::CommandBuffer cmdBuf = *m_impl->commandBuffers[m_impl->currentIndex];

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf;

	device->GetImpl().graphicsQueue.submit(submitInfo, nullptr);
	device->GetImpl().graphicsQueue.waitIdle();
}

void core::gpu::CommandBuffer::Impl::EndRendering()
{
	GetCommandBuffer(currentIndex).endRendering();
}

void core::gpu::CommandBuffer::Impl::BindPipeline(const core::gpu::Pipeline* pipeline)
{
	GetCommandBuffer(currentIndex).bindPipeline(
		vk::PipelineBindPoint::eGraphics,
		vk::Pipeline(pipeline->GetImpl().pipeline)
	);
}

core::gpu::CommandBuffer::CommandBuffer(const core::gpu::Device* device, const SCommandBufferCreateInfo& info)
{
	m_impl = std::make_unique<Impl>(*this, device, info);
}

core::gpu::CommandBuffer::~CommandBuffer() = default;

core::gpu::CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;
core::gpu::CommandBuffer& core::gpu::CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

void core::gpu::CommandBuffer::Impl::TransitionImageLayout(
	const core::gpu::Image* image,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout,
	vk::AccessFlags srcAccess,
	vk::AccessFlags dstAccess,
	vk::PipelineStageFlags srcStage,
	vk::PipelineStageFlags dstStage,
	bool isDepth)
{
	vk::ImageMemoryBarrier barrier{};
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->GetImpl().image;

	vk::ImageSubresourceRange subRange{};
	subRange.aspectMask = isDepth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
	subRange.baseMipLevel = 0;
	subRange.levelCount = 1;
	subRange.baseArrayLayer = 0;
	subRange.layerCount = 1;

	barrier.subresourceRange = subRange;

	GetCommandBuffer(currentIndex).pipelineBarrier(
		srcStage, dstStage, {}, {}, {}, { barrier }
	);
}

void core::gpu::CommandBuffer::Impl::ResolveImage(const core::gpu::Image* srcImage, const core::gpu::Image* dstImage, const core::gpu::Device* device)
{
	vk::ImageResolve resolveRegion{};
	vk::ImageSubresourceLayers subRange{};
	subRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	subRange.mipLevel = 0;
	subRange.baseArrayLayer = 0;
	subRange.layerCount = 1;
	resolveRegion.srcSubresource = subRange;
	vk::Offset3D offset = { 0, 0, 0 };
	resolveRegion.srcOffset = offset;

	vk::ImageSubresourceLayers dstSubresource{};
	dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	dstSubresource.mipLevel = 0;
	dstSubresource.baseArrayLayer = 0;
	dstSubresource.layerCount = 1;
	resolveRegion.dstSubresource = dstSubresource;

	vk::Offset3D dstOffset = { 0, 0, 0 };
	resolveRegion.dstOffset = dstOffset;

	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::Extent3D ext3D = { width, height, 1 };
	resolveRegion.extent = ext3D;

	GetCommandBuffer(currentIndex).resolveImage(
		srcImage->GetImpl().image, vk::ImageLayout::eTransferSrcOptimal,
		dstImage->GetImpl().image, vk::ImageLayout::eTransferDstOptimal,
		resolveRegion
	);
}

void core::gpu::CommandBuffer::Impl::CopyBuffer(const core::gpu::Buffer* srcBuffer, const core::gpu::Buffer* dstBuffer, size_t size)
{
	vk::BufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	GetCommandBuffer(currentIndex).copyBuffer(srcBuffer->GetImpl().buffer, dstBuffer->GetImpl().buffer, copyRegion);
}

uint32_t core::gpu::CommandBuffer::GetCount() const
{
	return m_impl->GetCount();
}

void core::gpu::CommandBuffer::Begin(uint32_t index)
{
	m_impl->Begin(index);
}

void core::gpu::CommandBuffer::End(uint32_t index)
{
	m_impl->End(index);
}

void core::gpu::CommandBuffer::BindPipeline(const core::gpu::Pipeline* pipeline)
{
	m_impl->BindPipeline(pipeline);
}

void core::gpu::CommandBuffer::BindVertexBuffer(const core::gpu::Buffer* buffer, size_t offset)
{
	m_impl->BindVertexBuffer(buffer, offset);
}

void core::gpu::CommandBuffer::BindIndexBuffer(const core::gpu::Buffer* buffer, size_t offset)
{
	m_impl->BindIndexBuffer(buffer, offset);
}

void core::gpu::CommandBuffer::SetViewport(float x, float y, const core::gpu::Device* device, float minDepth, float maxDepth)
{
	m_impl->SetViewport(x, y, device, minDepth, maxDepth);
}

void core::gpu::CommandBuffer::SetScissor(int32_t x, int32_t y, const core::gpu::Device* device)
{
	m_impl->SetScissor(x, y, device);
}

void core::gpu::CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	m_impl->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void core::gpu::CommandBuffer::BeginRendering(const core::gpu::Device* device, const core::gpu::Image* colorImageView, const core::gpu::Image* depthImageView)
{
	m_impl->BeginRendering(device, colorImageView, depthImageView);
}

void core::gpu::CommandBuffer::BeginRendering(
	const core::gpu::Device* device,
	const std::vector<RenderingAttachmentInfo>& colorAttachments,
	const DepthAttachmentInfo& depthAttachment)
{
	m_impl->BeginRendering(device, colorAttachments, depthAttachment);
}

void core::gpu::CommandBuffer::EndRendering()
{
	m_impl->EndRendering();
}

core::gpu::CommandBuffer::Impl& core::gpu::CommandBuffer::GetImpl() const
{
	return *m_impl;
}

void core::gpu::CommandBuffer::TransitionImageLayout(const core::gpu::Image* image, ImageLayout oldLayout, ImageLayout newLayout, bool isDepth)
{
	vk::AccessFlags srcAccess, dstAccess;
	vk::PipelineStageFlags srcStage, dstStage;
	vk::ImageLayout vkOldLayout, vkNewLayout;

	if (isDepth && oldLayout == ImageLayout::Undefined)
	{
		srcAccess = {};
		dstAccess = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		vkOldLayout = vk::ImageLayout::eUndefined;
		vkNewLayout = vk::ImageLayout::eDepthAttachmentOptimal;

		m_impl->TransitionImageLayout(image, vkOldLayout, vkNewLayout, srcAccess, dstAccess, srcStage, dstStage, isDepth);
		return;
	}

	switch (oldLayout)
	{
	case ImageLayout::Undefined: vkOldLayout = vk::ImageLayout::eUndefined; break;
	case ImageLayout::ColorAttachment: vkOldLayout = vk::ImageLayout::eColorAttachmentOptimal; break;
	case ImageLayout::TransferSrc: vkOldLayout = vk::ImageLayout::eTransferSrcOptimal; break;
	case ImageLayout::TransferDst: vkOldLayout = vk::ImageLayout::eTransferDstOptimal; break;
	case ImageLayout::Present: vkOldLayout = vk::ImageLayout::ePresentSrcKHR; break;
	case ImageLayout::ShaderReadOnly: vkOldLayout = vk::ImageLayout::eShaderReadOnlyOptimal; break;
	default: vkOldLayout = vk::ImageLayout::eUndefined;
	}

	switch (newLayout)
	{
	case ImageLayout::Undefined: vkNewLayout = vk::ImageLayout::eUndefined; break;
	case ImageLayout::ColorAttachment: vkNewLayout = vk::ImageLayout::eColorAttachmentOptimal; break;
	case ImageLayout::TransferSrc: vkNewLayout = vk::ImageLayout::eTransferSrcOptimal; break;
	case ImageLayout::TransferDst: vkNewLayout = vk::ImageLayout::eTransferDstOptimal; break;
	case ImageLayout::Present: vkNewLayout = vk::ImageLayout::ePresentSrcKHR; break;
	case ImageLayout::ShaderReadOnly: vkNewLayout = vk::ImageLayout::eShaderReadOnlyOptimal; break;
	default: vkNewLayout = vk::ImageLayout::eUndefined;
	}

	if (oldLayout == ImageLayout::Undefined && newLayout == ImageLayout::TransferDst)
	{
		srcAccess = {};
		dstAccess = vk::AccessFlagBits::eTransferWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == ImageLayout::ColorAttachment && newLayout == ImageLayout::TransferSrc)
	{
		srcAccess = vk::AccessFlagBits::eColorAttachmentWrite;
		dstAccess = vk::AccessFlagBits::eTransferRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == ImageLayout::TransferDst && newLayout == ImageLayout::Present)
	{
		srcAccess = vk::AccessFlagBits::eTransferWrite;
		dstAccess = vk::AccessFlagBits::eMemoryRead;
		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	else if (oldLayout == ImageLayout::Undefined && newLayout == ImageLayout::ColorAttachment)
	{
		srcAccess = {};
		dstAccess = vk::AccessFlagBits::eColorAttachmentWrite;
		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	else if (oldLayout == ImageLayout::ColorAttachment && newLayout == ImageLayout::ShaderReadOnly)
	{
		srcAccess = vk::AccessFlagBits::eColorAttachmentWrite;
		dstAccess = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == ImageLayout::ShaderReadOnly && newLayout == ImageLayout::TransferSrc)
	{
		srcAccess = vk::AccessFlagBits::eShaderRead;
		dstAccess = vk::AccessFlagBits::eTransferRead;
		srcStage = vk::PipelineStageFlagBits::eFragmentShader;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == ImageLayout::DepthStencilAttachment && newLayout == ImageLayout::ShaderReadOnly)
	{
		srcAccess = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		dstAccess = vk::AccessFlagBits::eShaderRead;
		srcStage = vk::PipelineStageFlagBits::eLateFragmentTests;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::runtime_error("Unsupported layout transition!");
	}

	m_impl->TransitionImageLayout(image, vkOldLayout, vkNewLayout, srcAccess, dstAccess, srcStage, dstStage, isDepth);
}

void core::gpu::CommandBuffer::ResolveImage(const core::gpu::Image* srcImage, const core::gpu::Image* dstImage, const core::gpu::Device* device)
{
	m_impl->ResolveImage(srcImage, dstImage, device);
}

void core::gpu::CommandBuffer::Impl::BlitImage(
	const core::gpu::Image* srcImage,
	const core::gpu::Image* dstImage,
	const core::gpu::Device* device)
{
	uint32_t width = device->GetImpl().swapchainExtent.width;
	uint32_t height = device->GetImpl().swapchainExtent.height;

	vk::ImageSubresourceLayers subRes{};
	subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
	subRes.mipLevel = 0;
	subRes.baseArrayLayer = 0;
	subRes.layerCount = 1;

	vk::ImageBlit region{};
	region.srcSubresource = subRes;
	region.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
	region.srcOffsets[1] = vk::Offset3D{ static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };
	region.dstSubresource = subRes;
	region.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
	region.dstOffsets[1] = vk::Offset3D{ static_cast<int32_t>(width), static_cast<int32_t>(height), 1 };

	GetCommandBuffer(currentIndex).blitImage(
		srcImage->GetImpl().image, vk::ImageLayout::eTransferSrcOptimal,
		dstImage->GetImpl().image, vk::ImageLayout::eTransferDstOptimal,
		region,
		vk::Filter::eLinear
	);
}

void core::gpu::CommandBuffer::BlitImage(
	const core::gpu::Image* srcImage,
	const core::gpu::Image* dstImage,
	const core::gpu::Device* device)
{
	m_impl->BlitImage(srcImage, dstImage, device);
}

void core::gpu::CommandBuffer::CopyBuffer(const core::gpu::Buffer* srcBuffer, const core::gpu::Buffer* dstBuffer, size_t size)
{
	m_impl->CopyBuffer(srcBuffer, dstBuffer, size);
}

void core::gpu::CommandBuffer::PushConstants(const core::gpu::Pipeline* pipeline,
	uint32_t stageFlags,
	uint32_t offset,
	uint32_t size,
	const void* pValues)
{
	m_impl->PushConstants(pipeline, stageFlags, offset, size, pValues);
}

void core::gpu::CommandBuffer::BuildAccelerationStructure(const core::gpu::AccelerationStructure* accelerationStructure)
{
	m_impl->BuildAccelerationStructure(accelerationStructure);
}

void core::gpu::CommandBuffer::AccelerationStructureBarrier()
{
	m_impl->AccelerationStructureBarrier();
}

void core::gpu::CommandBuffer::BindRayTracingPipeline(const core::gpu::Pipeline* pipeline)
{
	m_impl->BindRayTracingPipeline(pipeline);
}

void core::gpu::CommandBuffer::TraceRays(
	const core::gpu::Device* device,
	void* raygenSBT, uint32_t raygenOffset, uint32_t raygenStride,
	void* missSBT, uint32_t missOffset, uint32_t missStride, uint32_t missCount,
	void* hitSBT, uint32_t hitOffset, uint32_t hitStride, uint32_t hitCount,
	void* callableSBT, uint32_t callableOffset, uint32_t callableStride, uint32_t callableCount,
	uint32_t width, uint32_t height, uint32_t depth)
{
	m_impl->TraceRays(
		device,
		raygenSBT, raygenOffset, raygenStride,
		missSBT, missOffset, missStride, missCount,
		hitSBT, hitOffset, hitStride, hitCount,
		callableSBT, callableOffset, callableStride, callableCount,
		width, height, depth
	);
}