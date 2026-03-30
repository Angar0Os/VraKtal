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
#include <graphics/pass.h>
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
        glm::mat4 viewProjInverse;
        glm::vec4 viewPos;
        struct LightData
        {
            glm::vec4 position;
            glm::vec4 color;
            float     intensity;
            int       enabled;
            int       type;
            float     lightRadius;
        };
        LightData lights[MAX_LIGHTS];
        int       numLights;
        uint32_t  frameCount;
        alignas(4) uint32_t pad[3];
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
        std::unique_ptr<AccelerationStructure>      m_tlas;
        std::vector<std::pair<resources::Mesh*, glm::mat4>> m_meshInstances;
        std::vector<std::unique_ptr<AccelerationStructure>> m_tlasPerFrame;
        std::vector<Light>                                  m_lights;
        std::vector<std::unique_ptr<Pass>> m_passes;
        class GBufferPass*  m_gBufferPass  = nullptr;
        class LightingPass* m_lightingPass = nullptr;
        std::vector<std::unique_ptr<Buffer>> uniformBuffers;
        uint32_t  m_currentFrame;
        uint64_t  m_frameCounter;
        bool      m_running;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projMatrix;
        glm::vec3 m_cameraPosition;

        void CreateCommandBuffers();
        void BuildTLAS();
        void RebuildAccelerationStructures();
        void UpdateUniformBuffer(uint32_t frameIndex);
        void CreateUniformBuffers();
        void InitPasses();

    public:
        Renderer(core::Window& window, Device& device);
        ~Renderer();

        void SetCamera(const glm::mat4& view, const glm::mat4& projection);
        void PushMesh(resources::Mesh* mesh, const glm::mat4& transform);
        void PushLight(const resources::Light& light);
        void Render(core::gpu::Image* outputImage, ImageLayout outputLayout);
        void OnResize();
        void DrawScene(core::gpu::CommandBuffer* _cmd);
        void Cleanup();

        CommandBuffer* GetCurrentCommandBuffer();
        void Advance();

        core::gpu::AccelerationStructure* GetTLAS() const { return m_tlas.get(); }
        glm::mat4 GetViewMatrix()       { return m_viewMatrix;    }
        glm::mat4 GetProjectionMatrix() { return m_projMatrix;    }
        glm::vec3 GetCameraPosition()   { return m_cameraPosition; }
        uint32_t  GetCurrentFrame()     { return m_currentFrame;  }

        std::vector<std::pair<resources::Mesh*, glm::mat4>>* GetMeshInstances()
        {
            return &m_meshInstances;
        }

        template<typename T>
        T* GetPass(const std::string& name)
        {
            for (auto& p : m_passes)
                if (p->GetName() == name)
                    return dynamic_cast<T*>(p.get());
            return nullptr;
        }
    };
}

#endif //VRAKTAL_GRAPHICS_RENDERER_H