#ifndef VRAKTAL_CORE_GRAPHICS_MESH_IMPL_VULKAN_H
#define VRAKTAL_CORE_GRAPHICS_MESH_IMPL_VULKAN_H
#pragma once

#include <vulkan/vulkan.h>
#include <graphics/mesh.h>
#include <unordered_map>
#include <string_view>
#include <span>
#include <expected>
#include <string>

#include <core/renderContext.h>
#include "../core/gpu-details/vkTypes.h"

struct DescriptorAllocatorGrowable;

struct GLTFMaterial
{
	vkTypes::MaterialInstance* data;
};

struct GeoSurface
{
	uint32_t startIndex;
	uint32_t count;
	vkTypes::Bounds bounds;
	std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset
{
	std::string name;

	std::vector<GeoSurface> surfaces;
	vkTypes::GPUMeshBuffers meshBuffers;
};

class IRenderable
{
	virtual void Draw(const glm::mat4& topMatrix, vkTypes::DrawContext& ctx) = 0;
};

struct Node : public IRenderable
{
	std::weak_ptr<Node> parent;
	std::vector<std::shared_ptr<Node>> children;

	glm::mat4 localTransform;
	glm::mat4 worldTransform;

	void refreshTransform(const glm::mat4& parentMatrix)
	{
		worldTransform = parentMatrix * localTransform;
		for (auto c : children)
		{
			c->refreshTransform(worldTransform);
		}
	}

	virtual void Draw(const glm::mat4& topMatrix, vkTypes::DrawContext& ctx)
	{
		for (auto& c : children)
		{
			c->Draw(topMatrix, ctx);
		}
	}
};

struct MeshNode : Node
{
	std::shared_ptr<MeshAsset> mesh;

	virtual void Draw(const glm::mat4& topMatrix, vkTypes::DrawContext& ctx) override;
};

struct LoadedGLTF : public IRenderable
{
	std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
	std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
	std::unordered_map<std::string, vkTypes::AllocatedImage> images;
	std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

	std::vector<std::shared_ptr<Node>> topNodes;

	std::vector<VkSampler> samplers;

	DescriptorAllocatorGrowable* descriptorPool;

	vkTypes::AllocatedBuffer materialDataBuffer;

	core::rhi::RenderContext* owner;

	~LoadedGLTF() { ClearAll(); }

	virtual void Draw(const glm::mat4& topMatrix, vkTypes::DrawContext& ctx);

private:

	void ClearAll();
};

struct graphics::rhi::Mesh::Internal
{
	std::expected<std::shared_ptr<LoadedGLTF>, std::string> LoadGLTF(core::rhi::RenderContext* rCtx, std::string_view filePath);
	vkTypes::GPUMeshBuffers UploadMesh(std::span<uint32_t> indices, std::span<vkTypes::Vertex> vertices, core::rhi::RenderContext& rCtx);
};

#endif //VRAKTAL_CORE_GRAPHICS_MESH_IMPL_VULKAN_H
