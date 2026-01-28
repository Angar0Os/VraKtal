#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/image.h>

#include <core/gpu/texture.h>

#include <graphics/resources/object/mesh.h>
#include <graphics/resources/object/light.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace graphics
{
	struct PushConstants
	{
		glm::mat4 model;
	};

	class Renderer
	{
	private:
		core::Window& m_window;
		core::gpu::Device& m_device;

		std::vector<std::unique_ptr<core::gpu::CommandBuffer>> m_commandBuffers;
		std::unique_ptr<core::gpu::AccelerationStructure> m_tlas;

		std::vector<std::pair<resources::Mesh*, glm::mat4>> m_meshInstances;
		std::vector<std::unique_ptr<core::gpu::AccelerationStructure>> m_tlasPerFrame;
		std::vector<resources::Light> m_lights;

		uint32_t m_currentFrame;
		uint64_t m_frameCounter;
		bool m_running;

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projMatrix;
		glm::vec3 m_cameraPosition;

		void CreateCommandBuffers();
		void BuildTLAS();
		void RebuildAccelerationStructures();
		void UpdateUniformBuffer(uint32_t frameIndex);

	public:
		Renderer(core::Window& window, core::gpu::Device& device);
		~Renderer();

		void SetCamera(const glm::mat4& view, const glm::mat4& projection);
		void PushMesh(resources::Mesh* mesh, const glm::mat4& transform);
		void PushLight(const resources::Light& light);

		void Render(const core::gpu::Image* image, uint32_t imageIndex);
		void Cleanup();

		core::gpu::AccelerationStructure* GetTLAS() const { return m_tlas.get(); }

		glm::mat4 GetViewMatrix()		{ return m_viewMatrix;		};
		glm::mat4 GetProjectionMatrix()	{ return m_projMatrix;		};
		glm::vec3 GetCameraPosition()	{ return m_cameraPosition;	};

		std::vector<std::pair<resources::Mesh*, glm::mat4>>* GetMeshInstances() { return &m_meshInstances; };
	};
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H