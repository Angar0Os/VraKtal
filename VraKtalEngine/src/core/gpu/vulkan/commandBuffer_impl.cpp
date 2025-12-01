#include "../src/core/gpu/vulkan/commandBuffer_impl.h"

#include <core/gpu/accelerationStructure.h>

#include <stdexcept>

core::gpu::CommandBuffer::Impl::Impl(core::gpu::CommandBuffer& p, vk::raii::Device& dev,
	vk::raii::Queue& q, vk::raii::CommandPool& pool, const CommandBufferCreateInfo& info)
	: parent(p), device(dev), queue(q), commandPool(pool),
	commandBuffers(nullptr),
	isSingleTime(info.singleTime), currentIndex(0)
{
	if (info.count == 0)
	{
		throw std::runtime_error("CommandBuffer count cannot be zero");
	}

	vk::CommandBufferLevel level = (info.level == CommandBufferLevel::Primary)
		? vk::CommandBufferLevel::ePrimary
		: vk::CommandBufferLevel::eSecondary;

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = *commandPool;
	allocInfo.level = level;
	allocInfo.commandBufferCount = info.count;

	commandBuffers = vk::raii::CommandBuffers(device, allocInfo);
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

void core::gpu::CommandBuffer::Impl::BuildAccelerationStructure(void* accelerationStructure)
{
	if (!accelerationStructure)
	{
		throw std::runtime_error("Invalid acceleration structure handle (nullptr)");
	}

	auto* accelStruct = static_cast<AccelerationStructure*>(accelerationStructure);

	try {
		accelStruct->Build(&GetCommandBuffer(currentIndex));
	}
	catch (const std::exception& e) {
		throw std::runtime_error(std::string("Failed to build acceleration structure: ") + e.what());
	}
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

void core::gpu::CommandBuffer::Impl::BindRayTracingPipeline(void* pipeline)
{
	VkPipeline vkPipeline = reinterpret_cast<VkPipeline>(pipeline);

	if (!vkPipeline)
	{
		throw std::runtime_error("Invalid ray tracing pipeline handle");
	}

	GetCommandBuffer(currentIndex).bindPipeline(
		vk::PipelineBindPoint::eRayTracingKHR,
		vk::Pipeline(vkPipeline)
	);
}

void core::gpu::CommandBuffer::Impl::TraceRays(
	void* pipeline,
	void* raygenSBT, uint32_t raygenOffset, uint32_t raygenStride,
	void* missSBT, uint32_t missOffset, uint32_t missStride, uint32_t missCount,
	void* hitSBT, uint32_t hitOffset, uint32_t hitStride, uint32_t hitCount,
	void* callableSBT, uint32_t callableOffset, uint32_t callableStride, uint32_t callableCount,
	uint32_t width, uint32_t height, uint32_t depth)
{
	vk::StridedDeviceAddressRegionKHR raygenRegion{};
	if (raygenSBT)
	{
		vk::Buffer raygenBuffer = reinterpret_cast<VkBuffer>(raygenSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = raygenBuffer;

		raygenRegion.deviceAddress = device.getBufferAddress(addressInfo) + raygenOffset;
		raygenRegion.stride = raygenStride;
		raygenRegion.size = raygenStride;
	}

	vk::StridedDeviceAddressRegionKHR missRegion{};
	if (missSBT && missCount > 0)
	{
		vk::Buffer missBuffer = reinterpret_cast<VkBuffer>(missSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = missBuffer;

		missRegion.deviceAddress = device.getBufferAddress(addressInfo) + missOffset;
		missRegion.stride = missStride;
		missRegion.size = missStride * missCount;
	}

	vk::StridedDeviceAddressRegionKHR hitRegion{};
	if (hitSBT && hitCount > 0)
	{
		vk::Buffer hitBuffer = reinterpret_cast<VkBuffer>(hitSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = hitBuffer;

		hitRegion.deviceAddress = device.getBufferAddress(addressInfo) + hitOffset;
		hitRegion.stride = hitStride;
		hitRegion.size = hitStride * hitCount;
	}

	vk::StridedDeviceAddressRegionKHR callableRegion{};
	if (callableSBT && callableCount > 0)
	{
		vk::Buffer callableBuffer = reinterpret_cast<VkBuffer>(callableSBT);
		vk::BufferDeviceAddressInfo addressInfo{};
		addressInfo.buffer = callableBuffer;

		callableRegion.deviceAddress = device.getBufferAddress(addressInfo) + callableOffset;
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

void core::gpu::CommandBuffer::Impl::Submit(void* waitSemaphore, void* signalSemaphore, void* fence)
{
	if (currentIndex >= commandBuffers.size())
	{
		throw std::runtime_error("No command buffer has been begun");
	}

	vk::CommandBuffer cmdBuf = *commandBuffers[currentIndex];

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuf;

	vk::Semaphore vkWaitSemaphore = VK_NULL_HANDLE;
	vk::Semaphore vkSignalSemaphore = VK_NULL_HANDLE;
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	if (waitSemaphore)
	{
		vkWaitSemaphore = reinterpret_cast<VkSemaphore>(waitSemaphore);
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &vkWaitSemaphore;
		submitInfo.pWaitDstStageMask = &waitStage;
	}

	if (signalSemaphore)
	{
		vkSignalSemaphore = reinterpret_cast<VkSemaphore>(signalSemaphore);
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &vkSignalSemaphore;
	}

	vk::Fence vkFence = fence ? reinterpret_cast<VkFence>(fence) : nullptr;

	queue.submit(submitInfo, vkFence);
}

void core::gpu::CommandBuffer::Impl::PushConstants(void* pipelineLayout,
	uint32_t stageFlags,
	uint32_t offset,
	uint32_t size,
	const void* pValues)
{
	if (!pipelineLayout || !pValues)
	{
		throw std::runtime_error("Invalid push constants parameters!");
	}

	vk::PipelineLayout vkLayout = reinterpret_cast<VkPipelineLayout>(pipelineLayout);

	vk::ShaderStageFlags vkStageFlags;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Vertex))
		vkStageFlags |= vk::ShaderStageFlagBits::eVertex;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Fragment))
		vkStageFlags |= vk::ShaderStageFlagBits::eFragment;

	if (stageFlags & static_cast<uint32_t>(ShaderStageFlags::Compute))
		vkStageFlags |= vk::ShaderStageFlagBits::eCompute;

	GetCommandBuffer(currentIndex).pushConstants<uint8_t>(
		vkLayout,
		vkStageFlags,
		offset,
		vk::ArrayProxy<const uint8_t>(size, static_cast<const uint8_t*>(pValues))
	);
}

void core::gpu::CommandBuffer::Impl::SubmitAndWait()
{
	Submit();
	queue.waitIdle();
}

void core::gpu::CommandBuffer::Impl::BindVertexBuffer(void* buffer, size_t offset)
{
	vk::Buffer vkBuffer = reinterpret_cast<VkBuffer>(buffer);
	vk::DeviceSize vkOffset = static_cast<vk::DeviceSize>(offset);

	GetCommandBuffer(currentIndex).bindVertexBuffers(0, vkBuffer, vkOffset);
}

void core::gpu::CommandBuffer::Impl::BindIndexBuffer(void* buffer, size_t offset)
{
	vk::Buffer vkBuffer = reinterpret_cast<VkBuffer>(buffer);

	GetCommandBuffer(currentIndex).bindIndexBuffer(
		vkBuffer,
		static_cast<vk::DeviceSize>(offset),
		vk::IndexType::eUint32
	);
}

void core::gpu::CommandBuffer::Impl::BindDescriptorSets(void* pipelineLayout,
	void* descriptorSet,
	uint32_t firstSet)
{
	vk::PipelineLayout vkLayout = reinterpret_cast<VkPipelineLayout>(pipelineLayout);
	vk::DescriptorSet vkDescSet = reinterpret_cast<VkDescriptorSet>(descriptorSet);

	GetCommandBuffer(currentIndex).bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		vkLayout,
		firstSet,
		vkDescSet,
		nullptr
	);
}

void core::gpu::CommandBuffer::Impl::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	vk::Viewport viewport(x, y, width, height, minDepth, maxDepth);
	GetCommandBuffer(currentIndex).setViewport(0, viewport);
}

void core::gpu::CommandBuffer::Impl::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	vk::Rect2D scissor({ x, y }, { width, height });
	GetCommandBuffer(currentIndex).setScissor(0, scissor);
}

void core::gpu::CommandBuffer::Impl::DrawIndexed(uint32_t indexCount, uint32_t instanceCount,
	uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	GetCommandBuffer(currentIndex).drawIndexed(
		indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void core::gpu::CommandBuffer::Impl::BeginRendering(uint32_t width,
	uint32_t height,
	void* colorImageView,
	void* depthImageView)
{
	vk::ImageView vkColorView = reinterpret_cast<VkImageView>(colorImageView);
	vk::ImageView vkDepthView = reinterpret_cast<VkImageView>(depthImageView);

	vk::RenderingAttachmentInfo colorAttachment{};
	colorAttachment.imageView = vkColorView;
	colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.f);

	vk::RenderingAttachmentInfo depthAttachment{};
	depthAttachment.imageView = vkDepthView;
	depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.clearValue = vk::ClearDepthStencilValue(1.f, 0);

	vk::RenderingInfo info{};
	info.renderArea = vk::Rect2D({ 0, 0 }, { width, height });
	info.layerCount = 1;
	info.colorAttachmentCount = 1;
	info.pColorAttachments = &colorAttachment;
	info.pDepthAttachment = &depthAttachment;

	GetCommandBuffer(currentIndex).beginRendering(info);
}

void core::gpu::CommandBuffer::Impl::EndRendering()
{
	GetCommandBuffer(currentIndex).endRendering();
}

void core::gpu::CommandBuffer::Impl::BindPipeline(void* pipeline)
{
	VkPipeline vkPipeline = reinterpret_cast<VkPipeline>(pipeline);

	if (!vkPipeline)
	{
		throw std::runtime_error("Invalid pipeline handle");
	}

	GetCommandBuffer(currentIndex).bindPipeline(
		vk::PipelineBindPoint::eGraphics,
		vk::Pipeline(vkPipeline)
	);
}

core::gpu::CommandBuffer::CommandBuffer(void* device, void* queue, const CommandBufferCreateInfo& info)
{
	auto& vkDevice = *static_cast<vk::raii::Device*>(device);
	auto& vkQueue = *static_cast<vk::raii::Queue*>(queue);
	auto& vkCommandPool = *static_cast<vk::raii::CommandPool*>(info.commandPool);

	m_impl = std::make_unique<Impl>(*this, vkDevice, vkQueue, vkCommandPool, info);
}

core::gpu::CommandBuffer::~CommandBuffer() = default;

core::gpu::CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;
core::gpu::CommandBuffer& core::gpu::CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

void* core::gpu::CommandBuffer::GetHandle(uint32_t index) const
{
	return static_cast<void*>(
		const_cast<VkCommandBuffer*>(
			reinterpret_cast<const VkCommandBuffer*>(&(*m_impl->GetCommandBuffer(index)))
			)
		);
}

void core::gpu::CommandBuffer::Impl::TransitionImageLayout(
	void* image,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout,
	vk::AccessFlags srcAccess,
	vk::AccessFlags dstAccess,
	vk::PipelineStageFlags srcStage,
	vk::PipelineStageFlags dstStage,
	bool isDepth)
{
	vk::Image vkImage = reinterpret_cast<VkImage>(image);

	vk::ImageMemoryBarrier barrier{};
	barrier.srcAccessMask = srcAccess;
	barrier.dstAccessMask = dstAccess;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = vkImage;

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

void core::gpu::CommandBuffer::Impl::ResolveImage(void* srcImage, void* dstImage, uint32_t width, uint32_t height)
{
	vk::Image vkSrcImage = reinterpret_cast<VkImage>(srcImage);
	vk::Image vkDstImage = reinterpret_cast<VkImage>(dstImage);

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

	vk::Extent3D ext3D = { width, height, 1 };
	resolveRegion.extent = ext3D;

	GetCommandBuffer(currentIndex).resolveImage(
		vkSrcImage, vk::ImageLayout::eTransferSrcOptimal,
		vkDstImage, vk::ImageLayout::eTransferDstOptimal,
		resolveRegion
	);
}

void core::gpu::CommandBuffer::Impl::CopyBuffer(void* srcBuffer, void* dstBuffer, size_t size)
{
	vk::Buffer vkSrcBuffer = reinterpret_cast<VkBuffer>(srcBuffer);
	vk::Buffer vkDstBuffer = reinterpret_cast<VkBuffer>(dstBuffer);

	vk::BufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	GetCommandBuffer(currentIndex).copyBuffer(vkSrcBuffer, vkDstBuffer, copyRegion);
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

void core::gpu::CommandBuffer::Submit(void* waitSemaphore, void* signalSemaphore, void* fence)
{
	m_impl->Submit(waitSemaphore, signalSemaphore, fence);
}

void core::gpu::CommandBuffer::SubmitAndWait()
{
	m_impl->SubmitAndWait();
}

void core::gpu::CommandBuffer::BindPipeline(void* pipeline)
{
	m_impl->BindPipeline(pipeline);
}

void core::gpu::CommandBuffer::BindVertexBuffer(void* buffer, size_t offset)
{
	m_impl->BindVertexBuffer(buffer, offset);
}

void core::gpu::CommandBuffer::BindIndexBuffer(void* buffer, size_t offset)
{
	m_impl->BindIndexBuffer(buffer, offset);
}

void core::gpu::CommandBuffer::BindDescriptorSets(void* pipelineLayout, void* descriptorSet, uint32_t firstSet)
{
	m_impl->BindDescriptorSets(pipelineLayout, descriptorSet, firstSet);
}

void core::gpu::CommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	m_impl->SetViewport(x, y, width, height, minDepth, maxDepth);
}

void core::gpu::CommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	m_impl->SetScissor(x, y, width, height);
}

void core::gpu::CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	m_impl->DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void core::gpu::CommandBuffer::BeginRendering(uint32_t width, uint32_t height, void* colorImageView, void* depthImageView)
{
	m_impl->BeginRendering(width, height, colorImageView, depthImageView);
}

void core::gpu::CommandBuffer::EndRendering()
{
	m_impl->EndRendering();
}

core::gpu::CommandBuffer::Impl& core::gpu::CommandBuffer::GetImpl()
{
	return *m_impl;
}

const core::gpu::CommandBuffer::Impl& core::gpu::CommandBuffer::GetImpl() const
{
	return *m_impl;
}

void core::gpu::CommandBuffer::TransitionImageLayout(void* image, ImageLayout oldLayout, ImageLayout newLayout, bool isDepth)
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
	else
	{
		throw std::runtime_error("Unsupported layout transition!");
	}

	m_impl->TransitionImageLayout(image, vkOldLayout, vkNewLayout, srcAccess, dstAccess, srcStage, dstStage, isDepth);
}

void core::gpu::CommandBuffer::ResolveImage(void* srcImage, void* dstImage, uint32_t width, uint32_t height)
{
	m_impl->ResolveImage(srcImage, dstImage, width, height);
}

void core::gpu::CommandBuffer::CopyBuffer(void* srcBuffer, void* dstBuffer, size_t size)
{
	m_impl->CopyBuffer(srcBuffer, dstBuffer, size);
}

void core::gpu::CommandBuffer::PushConstants(void* pipelineLayout,
	uint32_t stageFlags,
	uint32_t offset,
	uint32_t size,
	const void* pValues)
{
	m_impl->PushConstants(pipelineLayout, stageFlags, offset, size, pValues);
}

void core::gpu::CommandBuffer::BuildAccelerationStructure(void* accelerationStructure)
{
	m_impl->BuildAccelerationStructure(accelerationStructure);
}

void core::gpu::CommandBuffer::AccelerationStructureBarrier()
{
	m_impl->AccelerationStructureBarrier();
}

void core::gpu::CommandBuffer::BindRayTracingPipeline(void* pipeline)
{
	m_impl->BindRayTracingPipeline(pipeline);
}

void core::gpu::CommandBuffer::TraceRays(
	void* pipeline,
	void* raygenSBT, uint32_t raygenOffset, uint32_t raygenStride,
	void* missSBT, uint32_t missOffset, uint32_t missStride, uint32_t missCount,
	void* hitSBT, uint32_t hitOffset, uint32_t hitStride, uint32_t hitCount,
	void* callableSBT, uint32_t callableOffset, uint32_t callableStride, uint32_t callableCount,
	uint32_t width, uint32_t height, uint32_t depth)
{
	m_impl->TraceRays(
		pipeline,
		raygenSBT, raygenOffset, raygenStride,
		missSBT, missOffset, missStride, missCount,
		hitSBT, hitOffset, hitStride, hitCount,
		callableSBT, callableOffset, callableStride, callableCount,
		width, height, depth
	);
}