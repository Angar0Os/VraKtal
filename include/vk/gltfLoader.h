#ifndef VRALTAL_VK_LOADER_H
#define VRALTAL_VK_LOADER_H
#pragma once

#include <vk/descriptors.h>
#include <vk/material.h>
#include <vk/buffer.h>

#include <unordered_map>
#include <filesystem>
#include <glm/glm.hpp>

using namespace vk;

class VulkanEngine;

struct Bounds
{
	glm::vec3 origin;
	float sphereRadius;
	glm::vec3 extents;
};

struct GLTFMaterial
{
	MaterialInstance data;
};

struct GeoSurface
{
	uint32_t startIndex;
	uint32_t count;
	Bounds bounds;
	std::shared_ptr<GLTFMaterial> material;
};

struct GPUMeshBuffers
{
	AllocatedBuffer* indexBuffer;
	AllocatedBuffer* vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
};

struct MeshAsset
{
	std::string name;

	std::vector<GeoSurface> surfaces;
	GPUMeshBuffers meshBuffers;
};

struct DrawContext;

class IRenderable
{
	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
};


struct Node : public IRenderable
{
	std::weak_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> children;

	glm::mat4 localTransform;
	glm::mat4 worldTransform;

	void RefreshTransform(const glm::mat4& parentMatrix)
	{
		worldTransform = parentMatrix * localTransform;
		for (auto c : children)
		{
			c->RefreshTransform(worldTransform);
		}
	}

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx)
	{
		for (auto& c : children)
		{
			c->Draw(topMatrix, ctx);
		}
	};
};

struct LoadedGLTF : public IRenderable
{
	std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
	std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
	std::unordered_map<std::string, AllocatedImage> images;
	std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

	std::vector<std::shared_ptr<Node>> topNodes;

	std::vector<VkSampler> samplers;

	DescriptorAllocatorGrowable descriptorPool;

	AllocatedBuffer* materialDataBuffer;


	explicit LoadedGLTF(VulkanEngine* engine, VulkanDevice* device, VulkanBuffer* buffer, VulkanImage* image, DescriptorAllocator* descriptor);

	virtual void Draw(const glm::mat4& topMatrix, DrawContext& ctx);
private:

	void ClearAll(VulkanEngine* engine, VulkanDevice* device, VulkanBuffer* buffer, VulkanImage* image, DescriptorAllocator* descriptor);
};

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanEngine* engine, std::string_view filePath);

#endif //VRALTAL_VK_LOADER_H