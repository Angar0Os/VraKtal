#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/buffer.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/image.h>

#include <graphics/resources/scene.h>
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

		std::shared_ptr<resources::Scene> m_scene;

		uint32_t m_currentFrame;
		uint64_t m_frameCounter;
		bool m_running;

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projMatrix;
		glm::vec3 m_cameraPosition;

		void CreateCommandBuffers();
		void CreateMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh);
		void UpdateUniformBuffer(uint32_t frameIndex, const std::shared_ptr<resources::object::Material>& material);
		void RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex);

		void CreateRTMeshBuffers(std::shared_ptr<resources::object::Mesh> mesh);
		void CreateBLAS(resources::object::Mesh* mesh);
		void BuildTLAS();
		void RebuildAccelerationStructures();
		void UpdateCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position);

	public:
		Renderer(core::Window& window, core::gpu::Device& device);
		~Renderer();

		void SetScene(std::shared_ptr<resources::Scene> scene);
		void UpdateCameraFromScene();
		void DrawFrame();
		void Cleanup();

		void EnableRayTracing();
		void DisableRayTracing();
		bool IsRayTracingEnabled() const { return m_rayTracingEnabled; }

		core::gpu::AccelerationStructure* GetTLAS() const { return m_tlas.get(); }

		std::shared_ptr<resources::Scene> GetScene() { return m_scene; }
	};
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H