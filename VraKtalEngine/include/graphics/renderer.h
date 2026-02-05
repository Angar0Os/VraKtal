#ifndef VRAKTAL_GRAPHICS_RENDERER_H
#define VRAKTAL_GRAPHICS_RENDERER_H
#pragma once

#include <core/window.h>
#include <core/gpu/accelerationStructure.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/descriptorSetLayout.h>
#include <core/gpu/device.h>
#include <core/gpu/image.h>
#include <core/gpu/pipeline.h>
#include <core/gpu/texture.h> 

#include <graphics/resources/object/mesh.h>
#include <graphics/resources/object/light.h>

#include <glm/glm.hpp>

#include <memory>
#include <vector>

using namespace core;
using namespace core::gpu;
using namespace graphics::resources;


namespace graphics
{
	constexpr int MAX_LIGHTS = 10;

	struct UniformBufferObject
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 lightSpaceMatrix;
		glm::vec3 viewPos;

		struct LightData
		{
			glm::vec3 position;
			glm::vec3 color;
			float intensity;
			int enabled;
			int type;
			float lightRadius;
		};

		LightData lights[MAX_LIGHTS];
		int numLights;
		uint32_t frameCount;
		alignas(4) uint32_t pad[7];
	};

	struct PushConstants
	{
		glm::mat4 model;
	};

	class Renderer
	{
	private:
		Window& m_window;
		Device& m_device;

		std::vector<std::unique_ptr<CommandBuffer>> m_commandBuffers;
		std::unique_ptr<AccelerationStructure> m_tlas;

		std::vector<std::pair<resources::Mesh*, glm::mat4>> m_meshInstances;
		std::vector<std::unique_ptr<AccelerationStructure>> m_tlasPerFrame;
		std::vector<Light> m_lights;

		std::unique_ptr<Pipeline> graphicsPipeline = nullptr;
		std::unique_ptr<DescriptorSetLayout>	descriptorSetLayout = nullptr;

		std::vector<std::unique_ptr<DescriptorSet>> graphicsDescriptorSets;

		uint32_t m_currentFrame;
		uint64_t m_frameCounter;
		bool m_running;

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projMatrix;
		glm::vec3 m_cameraPosition;

		std::unique_ptr<Image>	colorImage;
		std::unique_ptr<Image>	depthImage;

		std::unique_ptr<core::gpu::Texture> colorTexture;
		std::unique_ptr<core::gpu::Texture> depthTexture;

		std::vector<std::unique_ptr<Buffer>> uniformBuffers;

		void CreateCommandBuffers();
		void BuildTLAS();
		void RebuildAccelerationStructures();
		void UpdateUniformBuffer(uint32_t frameIndex);

		void CreateGraphicsPipeline();
		void CreateDescriptorSetLayout();
		void CreateGraphicsDescriptorSet();
		void CreateUniformBuffers();

		void CreateColorImage();
		void CreateDepthImage();
	public:
		Renderer(core::Window& window, Device& device);
		~Renderer();

		void SetCamera(const glm::mat4& view, const glm::mat4& projection);
		void PushMesh(resources::Mesh* mesh, const glm::mat4& transform);
		void PushLight(const resources::Light& light);

		void Render(const core::gpu::Image* image, uint32_t imageIndex);
		void Cleanup();

		core::gpu::AccelerationStructure* GetTLAS() const { return m_tlas.get(); }

		glm::mat4 GetViewMatrix() { return m_viewMatrix; };
		glm::mat4 GetProjectionMatrix() { return m_projMatrix; };
		glm::vec3 GetCameraPosition() { return m_cameraPosition; };

		std::vector<std::pair<resources::Mesh*, glm::mat4>>* GetMeshInstances() { return &m_meshInstances; };
	};
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H