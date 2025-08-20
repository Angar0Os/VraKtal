#include "material_impl_vulkan.h"
#include "../core/gpu/pipeline_impl_vulkan.h"
#include "../core/gpu-details/vkInitializers.h"

#include <iostream>


void GLTFMetallic_Roughness::BuildPipelines(core::rhi::RenderContext& renderContext)
{
	VkShaderModule meshFragShader;
	if (!utils::LoadShaderModule("../bin/assets/shaders/mesh.frag.spv", renderContext.GetInternal().device, &meshFragShader))
	{
		std::cout << "Error when building the triangle fragment shader module" << std::endl;
	}

	VkShaderModule meshVertexShader;
	if (!utils::LoadShaderModule("../bin/assets/shaders/mesh.vert.spv", renderContext.GetInternal().device, &meshVertexShader))
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

	materialLayout = layoutBuilder.Build(renderContext.GetInternal().device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	// TODO : Trouver un moyen d'accéder à un de mes descriptors. Je pense que tout comme les pipelines, il doit y avoir un currentDescriptor ?
	
	//std::vector<VkDescriptorSetLayout> layouts = { renderContext.GetInternal().d};

	VkPipelineLayoutCreateInfo meshLayoutInfo = core::gpu_detail::PipelineLayoutCreateInfo();
	meshLayoutInfo.setLayoutCount = 2;
	//meshLayoutInfo.pSetLayouts = layouts;
	meshLayoutInfo.pPushConstantRanges = &matrixRange;
	meshLayoutInfo.pushConstantRangeCount = 1;

	VkPipelineLayout newLayout;
	vkCreatePipelineLayout(renderContext.GetInternal().device, &meshLayoutInfo, nullptr, &newLayout);

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;

	// TODO : On peut avoir plusieurs pipeline de rendu ? Si oui, il faut trouver le bon et faire toutes ces actions
	// TODO : Il doit probablement y avoir un currentPipeline dans ma currentFrame, à creuser 
	
	// renderContext.GetInternal().pipeline->GetInternal().SetShaders(meshVertexShader, meshFragShader);
	// renderContext.GetInternal().pipeline->GetInternal().SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// renderContext.GetInternal().pipeline->GetInternal().SetPolygonMode(VK_POLYGON_MODE_FILL);
	// renderContext.GetInternal().pipeline->GetInternal().SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	// renderContext.GetInternal().pipeline->GetInternal().SetMultisamplingNone();
	// renderContext.GetInternal().pipeline->GetInternal().DisableBlending();
	// renderContext.GetInternal().pipeline->GetInternal().EnableDepthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
	//
	// renderContext.GetInternal().pipeline->GetInternal().SetColorAttachmentFormat(renderContext.GetInternal().drawImage.imageFormat);
	// renderContext.GetInternal().pipeline->GetInternal().SetDepthFormat(renderContext.GetInternal().depthImage.imageFormat);
	//
	// renderContext.GetInternal().pipeline->GetInternal().PipelineLayout = newLayout;
	//
	// opaquePipeline.pipeline = renderContext.GetInternal().pipeline->GetInternal().BuildPipeline(renderContext.GetInternal().device);
	//
	// renderContext.GetInternal().pipeline->GetInternal().EnableBlendingAdditive();
	// renderContext.GetInternal().pipeline->GetInternal().EnableDepthtest(false, VK_COMPARE_OP_LESS_OR_EQUAL);
	//
	// transparentPipeline.pipeline = renderContext.GetInternal().pipeline->GetInternal().BuildPipeline(renderContext.GetInternal().device);

	vkDestroyShaderModule(renderContext.GetInternal().device, meshFragShader, nullptr);
	vkDestroyShaderModule(renderContext.GetInternal().device, meshVertexShader, nullptr);
}

void GLTFMetallic_Roughness::ClearResources(VkDevice device)
{
	vkDestroyDescriptorSetLayout(device, materialLayout, nullptr);
	vkDestroyPipelineLayout(device, transparentPipeline.layout, nullptr);

	vkDestroyPipeline(device, transparentPipeline.pipeline, nullptr);
	vkDestroyPipeline(device, opaquePipeline.pipeline, nullptr);
}

MaterialInstance GLTFMetallic_Roughness::WriteMaterial(VkDevice device, vkTypes::MaterialPass pass, const MaterialResources& resources, DescriptorAllocatorGrowable& descriptorAllocator)
{
	MaterialInstance matData;
	matData.passType = pass;
	if (pass == vkTypes::MaterialPass::Transparent)
	{
		matData.pipeline = &transparentPipeline;
	}
	else
	{
		matData.pipeline = &opaquePipeline;
	}

	matData.materialSet = descriptorAllocator.Allocate(device, materialLayout);

	writer.Clear();
	writer.WriteBuffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.WriteImage(1, resources.colorImage.imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.WriteImage(2, resources.metalRoughImage.imageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer.UpdateSet(device, matData.materialSet);

	return matData;
}

graphics::rhi::Material::Material()
	: m_Internal(new Internal)
{
}