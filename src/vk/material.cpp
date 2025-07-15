#include <vk/material.h>
#include <vk/image.h>
#include <vk/descriptors.h>
#include <vk/device.h>
#include <vk/pipeline.h>
#include <vk/swapchain.h>
#include <vkEngine.h>

using namespace vk;

void GLTFMetallic_Roughness::BuildPipelines(VulkanEngine* engine, VulkanDevice* device, VulkanPipeline* pipeline, VulkanSwapchain* swapchain)
{
	VkShaderModule meshFragShader;
	if (!vkutils::LoadShaderModule("assets/shaders/mesh.frag.spv", device->GetVkDevice(), &meshFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}

	VkShaderModule meshVertexShader;
	if (!vkutils::LoadShaderModule("assets/shaders/mesh.vert.spv", device->GetVkDevice(), &meshVertexShader))
	{
		std::cout << "Error when building the triangle vertex shader module" << std::endl;
	}

	VkPushConstantRange matrixRange{};
	matrixRange.offset = 0;
	matrixRange.size = sizeof(GPUDrawPushConstants);
	matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	layoutBuilder.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	materialLayout = layoutBuilder.Build(device->GetVkDevice(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	VkDescriptorSetLayout layouts[] = { gpuSceneDataDescriptorLayout, materialLayout };

	VkPipelineLayoutCreateInfo mesh_layout_info = pipeline->PipelineLayoutCreateInfo();
	mesh_layout_info.setLayoutCount = 2;
	mesh_layout_info.pSetLayouts = layouts;
	mesh_layout_info.pPushConstantRanges = &matrixRange;
	mesh_layout_info.pushConstantRangeCount = 1;

	VkPipelineLayout newLayout;
	vkCreatePipelineLayout(device->GetVkDevice(), &mesh_layout_info, nullptr, &newLayout);

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;

	VulkanPipeline pipelineBuilder;
	pipelineBuilder.SetShaders(meshVertexShader, meshFragShader);
	pipelineBuilder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.SetPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.SetMultisamplingNone();
	pipelineBuilder.DisableBlending();
	pipelineBuilder.EnableDepthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);

	pipelineBuilder.SetColorAttachmentFormat(swapchain->GetDrawImage()->imageFormat);
	pipelineBuilder.SetDepthFormat(swapchain->GetDepthImage()->imageFormat);

	pipelineBuilder._pipelineLayout = newLayout;

	opaquePipeline.pipeline = pipelineBuilder.BuildPipeline(device->GetVkDevice());

	pipelineBuilder.EnableBlendingAdditive();
	pipelineBuilder.EnableDepthtest(false, VK_COMPARE_OP_LESS_OR_EQUAL);

	transparentPipeline.pipeline = pipelineBuilder.BuildPipeline(device->GetVkDevice());

	vkDestroyShaderModule(device->GetVkDevice(), meshFragShader, nullptr);
	vkDestroyShaderModule(device->GetVkDevice(), meshVertexShader, nullptr);
}

void GLTFMetallic_Roughness::ClearResources(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
	vkDestroyPipelineLayout(device, transparentPipeline.layout, nullptr);

	vkDestroyPipeline(device, transparentPipeline.pipeline, nullptr);
	vkDestroyPipeline(device, opaquePipeline.pipeline, nullptr);
}

MaterialInstance GLTFMetallic_Roughness::WriteMaterial(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator)
{
	MaterialInstance matData;
	matData.passType = pass;
	if (pass == MaterialPass::Transparent)
	{
		matData.pipeline = &transparentPipeline;
	}
	else
	{
		matData.pipeline = &opaquePipeline;
	}

	matData.materialSet = descriptorAllocator.Allocate(device, materialLayout);

	writer->Clear();
	writer->WriteBuffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer->WriteImage(1, resources.colorImage->imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer->WriteImage(2, resources.metalRoughImage->imageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer->UpdateSet(device, matData.materialSet);

	return matData;
}