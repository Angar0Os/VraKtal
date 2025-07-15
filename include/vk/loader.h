#ifndef VRALTAL_VK_LOADER_H
#define VRALTAL_VK_LOADER_H
#pragma once

#include <vk/descriptors.h>
#include <vk/material.h>

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

struct MeshAsset
{
	std::string name;

	std::vector<GeoSurface> surfaces;
	GLTFMetallic_Roughness::GPUDrawPushConstants meshBuffers;
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
	}
};

#endif //VRALTAL_VK_LOADER_H