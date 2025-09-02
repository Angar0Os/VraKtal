#include "../src/core/gpu/window_impl_vulkan.h"
#include "../src/core/gpu/gpuDevice_impl_glfw_vulkan.h"
#include "../src/core/gpu/commandBuffer_impl_vulkan.h"

#include "../src/graphics/renderer_impl_vulkan.h"
#include "../src/graphics/meshRenderer.h"
#include "../src/graphics/loaders/gltfLoader.h"
#include "../src/graphics/loaders/textureLoader.h"
#include "../src/graphics/resources/material.h"

#include <core/gpu/renderingInfo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

#pragma comment(lib, "VraKtalEngine_Debug.lib")

using namespace rhi::vulkan;
using namespace rhi::core::gpu;
using namespace graphics;
using namespace graphics::scene;
using namespace graphics::loaders;
using namespace graphics::resources;

int main()
{
    WindowVulkan window(800, 600, "Textured Mesh");
    GpuDeviceVulkan device(window);

    device.WrapSwapchainImages();

    MeshRenderer meshRenderer(device);
    RendererVulkan renderer(&meshRenderer);

    Scene scene = LoadScene("../bin/assets/meshes/house.glb");

    TextureLoader texLoader(device);
    std::vector<TextureGpu> gpuTextures;
    texLoader.LoadSceneTextures(scene, gpuTextures);

    meshRenderer.CreateDescriptorPool((uint32_t)scene.materials.size());

    std::vector<VkDescriptorSet> materialSets;
    materialSets.reserve(scene.materials.size());

    for (auto& mat : scene.materials) {
        int texIndex = mat.baseColorTexture;
        if (texIndex < 0 || texIndex >= (int)gpuTextures.size()) {
            texIndex = 0;
        }
        auto& t = gpuTextures[texIndex];
        materialSets.push_back(meshRenderer.CreateDescriptorSet(t.view, t.sampler));
    }

    meshRenderer.SetMaterialDescriptorSets(materialSets);

    std::vector<GpuMesh> gpuMeshes;
    gpuMeshes.reserve(scene.meshes.size());
    for (auto& m : scene.meshes) {
        gpuMeshes.push_back(meshRenderer.UploadMesh(m));
    }

    std::vector<CommandBufferVulkan*> commandBuffers;
    for (int i = 0; i < 2; ++i) {
        commandBuffers.push_back(new CommandBufferVulkan(device));
    }

    uint32_t currentFrame = 0;

    glm::mat4 view = glm::lookAt(glm::vec3(-10, 20, 20), glm::vec3(0, 10, 0), glm::vec3(0, 1, 0));

    auto size = window.Size();
    float aspect = static_cast<float>(size.first) / static_cast<float>(size.second);

    glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(60.0f), aspect, 0.1f, 100.0f);
    proj[1][1] *= -1;

    while (!window.ShouldClose())
    {
        window.PollEvents();

        uint32_t imageIndex;
        if (!device.BeginFrame(imageIndex)) 
        {
            continue;
        }

        CommandBufferVulkan& commandBuffer = *commandBuffers[currentFrame];
        commandBuffer.Begin();

        RenderingInfo info;
        info.width = device.SwapExtent().width;
        info.height = device.SwapExtent().height;

        RenderingAttachment color{};
        color.image = device.GetSwapchainImage(imageIndex);
        color.clearValue = { 0.1f, 0.1f, 0.15f, 1.f };
        color.loadOp = LoadOp::Clear;
        color.storeOp = StoreOp::Store;
        info.colorAttachments.push_back(color);

        renderer.Render(commandBuffer, info, imageIndex, gpuMeshes, view, proj);

        commandBuffer.End();
        device.EndFrame(imageIndex, commandBuffer.GetNative());

        currentFrame = (currentFrame + 1) % 2;
    }

    device.WaitIdle();

    for (auto& g : gpuMeshes)
    {
        meshRenderer.DestroyMesh(g);
    }
    for (auto* cmd : commandBuffers)
    {
        delete cmd;
    }

    std::cout << "Done\n";
    return 0;
}