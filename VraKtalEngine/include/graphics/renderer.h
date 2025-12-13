#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/buffer.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/image.h>

#include <graphics/resources/object/staticMesh.h>
#include <graphics/resources/object/camera.h>
#include <graphics/resources/object/light.h>
#include <graphics/resources/object/mesh.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>

namespace graphics
{
	struct PushConstants
	{
		glm::mat4 model;
	};

	struct RenderableStaticMesh
	{
		resources::object::StaticMesh* staticMesh;
		glm::mat4 transform;
	};

	struct LightData
	{
		glm::vec3 position;
		glm::vec3 color;
		float intensity;
		float radius;
		bool enabled;
	};

	class Renderer
	{
	private:
		struct MeshBuffers
		{
			std::unique_ptr<core::gpu::Buffer> vertexBuffer;
			std::unique_ptr<core::gpu::Buffer> indexBuffer;
			uint32_t indexCount = 0;
		};

		struct RTMeshData
		{
			std::unique_ptr<core::gpu::Buffer> rtVertexBuffer;
			std::unique_ptr<core::gpu::Buffer> rtIndexBuffer;
			std::unique_ptr<core::gpu::AccelerationStructure> blas;
		};

		core::Window& m_window;
		core::gpu::Device& m_device;

		std::vector<std::unique_ptr<core::gpu::CommandBuffer>> m_commandBuffers;
		std::unordered_map<resources::object::Mesh*, MeshBuffers> m_meshBuffers;

		std::unordered_map<resources::object::Mesh*, RTMeshData> m_rtMeshData;
		std::unique_ptr<core::gpu::AccelerationStructure> m_tlas;
		bool m_rayTracingEnabled = false;

		std::vector<RenderableStaticMesh> m_staticMeshes;
		std::vector<LightData> m_lights;
		glm::mat4 m_viewMatrix;
		glm::mat4 m_projMatrix;
		glm::vec3 m_cameraPosition;

		uint32_t m_currentFrame;
		uint64_t m_frameCounter;
		bool m_running;

		void CreateCommandBuffers();
		void CreateMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh);
		void UpdateUniformBuffer(uint32_t frameIndex, const std::shared_ptr<resources::object::Material>& material);

		void CreateRTMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh);
		void CreateBLAS(resources::object::Mesh* mesh);
		void BuildTLAS();
		void RebuildAccelerationStructures();

	public:
		Renderer(core::Window& window, core::gpu::Device& device);
		~Renderer();

		void PushObject(resources::object::StaticMesh& staticMesh, const glm::mat4& transform);
		void PushLight(const resources::object::Light& light);
		void SetActiveCamera(resources::object::Camera& camera, const glm::mat4& transform);

		void DrawFrame(const core::gpu::Image* swapchainImage, uint32_t frameIndex, uint32_t imageIndex);

		void Cleanup();

		void EnableRayTracing();
		void DisableRayTracing();
		bool IsRayTracingEnabled() const { return m_rayTracingEnabled; }

		core::gpu::AccelerationStructure* GetTLAS() const { return m_tlas.get(); }
	};
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H