#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/buffer.h>

#include <graphics/resources/scene.h>
#include <graphics/resources/mesh.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>

namespace graphics
{
	class Renderer
	{
	public:
		using ImGuiCallback = std::function<void()>;

	private:
		struct MeshBuffers
		{
			std::unique_ptr<core::gpu::Buffer> vertexBuffer;
			std::unique_ptr<core::gpu::Buffer> indexBuffer;
			uint32_t indexCount = 0;
		};

		core::Window& m_window;
		core::gpu::Device& m_device;

		std::vector<std::unique_ptr<core::gpu::CommandBuffer>> m_commandBuffers;
		std::unordered_map<resources::Mesh*, MeshBuffers> m_meshBuffers;

		std::shared_ptr<resources::Scene> m_scene;

		ImGuiCallback m_imguiCallback;

		uint32_t m_currentFrame;
		uint64_t m_frameCounter;
		bool m_running;

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projMatrix;
		glm::vec3 m_cameraPosition;

		void CreateCommandBuffers();
		void CreateMeshBuffers(std::shared_ptr<resources::Mesh> mesh);
		void UpdateUniformBuffer(uint32_t frameIndex);
		void RecordCommandBuffer(uint32_t frameIndex, uint32_t imageIndex);

	public:
		Renderer(core::Window& window, core::gpu::Device& device);
		~Renderer();

		void SetScene(std::shared_ptr<resources::Scene> scene);
		void UpdateCamera(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& position);
		void DrawFrame();
		void Cleanup();

		void SetImGuiCallback(ImGuiCallback callback) { m_imguiCallback = callback; }

		std::shared_ptr<resources::Scene> GetScene() { return m_scene; }
	};
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H