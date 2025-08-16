#ifndef VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#define VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
#pragma once

#include "../vkb/VkBootstrap.h"

#include <core/renderContext.h>
#include <core/gpu/descriptor.h>
#include <core/gpu/pipeline.h>
#include <core/gpu/image.h>

#include <graphics/mesh.h>
#include <graphics/material.h>

#include "gpu/commandBuffer_impl_vulkan.h"
#include "../demo/inputManager.h"
#include "../demo/camera.h"
#include "../graphics/mesh_impl_vulkan.h"
#include "../vkTypes.h"

#include <vma/vk_mem_alloc.h>

#include <iostream>
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

	std::unique_ptr<core::rhi::gpu::CommandBuffer> commandBuffer;
	std::unique_ptr<core::rhi::gpu::Descriptor> descriptor;
	std::unique_ptr<core::rhi::gpu::Pipeline> pipeline;
	std::unique_ptr<graphics::rhi::Material> material;
	std::unique_ptr<graphics::rhi::Mesh> meshLoader;
	std::unique_ptr<core::rhi::Image> imageLoader;


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

	vkTypes::AllocatedImage drawImage;
	vkTypes::AllocatedImage depthImage;

	vkTypes::AllocatedImage whiteImage;
	vkTypes::AllocatedImage blackImage;
	vkTypes::AllocatedImage greyImage;
	vkTypes::AllocatedImage errorCheckerboardImage;
	vkTypes::GPUMeshBuffers rectangle;
	vkTypes::GPUSceneData sceneData;

	DrawContext drawCommands;

	VkSampler defaultSamplerLinear;
	VkSampler defaultSamplerNearest;

	FrameData& GetCurrentFrame();
	FrameData& GetLastFrame();

	void CreateSwapchain(uint32_t width, uint32_t height);
	VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags);
	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
	VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	VkPresentInfoKHR PresentInfo();
	VkRenderingAttachmentInfo DepthAttachmentInfo(VkImageView view, VkImageLayout layout);
	VkRenderingAttachmentInfo AttachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout);
	VkRenderingInfo RenderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);

	void InitDefaultData();
	void InitRenderable();
	void Run();

	void ResizeSwapchain();
	void DestroySwapchain();

	void UpdateScene();
	void DrawMain(VkCommandBuffer cmd);
	void DrawGeometry(VkCommandBuffer cmd);
	void Draw();

	std::unordered_map<std::string, std::shared_ptr<LoadedGLTF>> LoadedScenes;

	demo::Camera* camera;

	VkExtent2D drawExtent;
	float renderScale{ 1.0f };

	bool IsInitialized{ false };
	bool ResizeRequested{ false };
	bool FreezeRendering{ false };

	int currentBackgroundEffect{ 0 };

private:
	RenderContext* m_parent;
};

#endif //VRAKTAL_CORE_RENDER_CONTEXT_IMPL_GLFW_VULKAN_H
