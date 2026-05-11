#include <core/manager/materialInstanceManager.h>
#include <factory/materialFactory.h>

#include <graphics/renderer.h>

#include <graphics/renderPass/gBufferPass.h>

materialInstanceManager::materialInstanceManager(graphics::Renderer& _renderer, core::gpu::Device& _device) : m_device(_device) , m_renderer(_renderer) {}

materialInstanceManager::~materialInstanceManager() {}

std::shared_ptr<graphics::resources::MaterialInstance> materialInstanceManager::CreateMaterialInstance(graphics::assets::Material& _material)
{
	auto matInstance = factory::MaterialFactory::CreateMaterialInstance(m_device, _material, GetMaterialDescriptorSetLayout(_material.materialType));
	m_materialInstanceMap[_material.name];
	m_materialInstanceMap[_material.name].push_back(matInstance);
	return matInstance;
}

void materialInstanceManager::UpdateMaterialInstancesOf(std::string _MaterialName)
{
}

const core::gpu::DescriptorSetLayout* materialInstanceManager::GetMaterialDescriptorSetLayout(graphics::MaterialType _type)
{
	switch (_type)
	{
	case graphics::MaterialType::PBR:
		return m_renderer.GetPass<graphics::GBufferPass>("GBuffer")->GetMaterialLayout();
	case graphics::MaterialType::Unlit:
		return nullptr;
		break;
	case graphics::MaterialType::Skybox:
		return nullptr;
		break;
	default:
		return nullptr;
		break;
	}
}
