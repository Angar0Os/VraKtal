#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include "../vkb/VkBootstrap.h"

#include <core/renderContext.h>

#include "../graphics/mesh_impl_vulkan.h"
#include "../core/gpu-details/vkTypes.h"

#include <vma/vk_mem_alloc.h>

#include <array>
#include <vector>
#include <memory>


struct GLFWwindow;
struct DescriptorAllocatorGrowable;

struct FrameData
{
	VkSemaphore swapchainSemaphore, renderSemaphore;
	VkFence renderFence;

	VkCommandPool commandPool;
	VkCommandBuffer mainCommandBuffer;

	vkTypes::DeletionQueue deletionQueue;
	std::unique_ptr<DescriptorAllocatorGrowable> frameDescriptors;
};

struct core::rhi::RenderContext::Internal
{
	Internal(RenderContext* parent);
	~Internal() = default;

	GLFWwindow* window;

	vkb::Instance instance;
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;
	vkTypes::DeletionQueue mainDeletionQueue;

	bool useValidationLayers = false;

	static constexpr unsigned int FRAME_OVERLAP = 2;

	int frameNumber{ 0 };

	VkDevice device;
	VkPhysicalDevice chosenGPU;
	VmaAllocator allocator;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	VkFence immFence;
	VkCommandBuffer immCommandBuffer;
	VkCommandPool immCommandPool;

	VkExtent2D windowExtent;

	FrameData frames[FRAME_OVERLAP];

	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	uint32_t swapchainImageIndex;

	vkTypes::AllocatedImage drawImage;
	vkTypes::AllocatedImage depthImage;

	VkSampler defaultSamplerLinear;
	VkSampler defaultSamplerNearest;

	FrameData& GetCurrentFrame();
	FrameData& GetLastFrame();

	void CreateSwapchain(uint32_t width, uint32_t height);

	void ResizeSwapchain();
	void DestroySwapchain();

	vkTypes::AllocatedBuffer CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void DestroyBuffer(const vkTypes::AllocatedBuffer buffer);
	void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	bool IsInitialized{ false };
	bool ResizeRequested{ false };
	
private:
	RenderContext* m_parent;
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
