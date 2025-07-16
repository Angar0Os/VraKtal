#include <vk/buffer.h>

using namespace vk;

AllocatedBuffer VulkanBuffer::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	AllocatedBuffer newBuffer;

	vmaCreateBuffer(_vkImage.GetAllocator(), &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info);

	return newBuffer;
}

void VulkanBuffer::Destroy(AllocatedBuffer& buffer)
{
	vmaDestroyBuffer(_vkImage.GetAllocator(), buffer.buffer, buffer.allocation);
}