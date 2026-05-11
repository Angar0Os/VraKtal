#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace graphics{
	namespace resources{
		struct MaterialInstance;
	}
	namespace assets {
		struct Material;
	}
	class Renderer;
}

namespace core::gpu {
	class Device;
	class DescriptorSetLayout;
}

namespace graphics {
	enum class MaterialType;
}

class materialInstanceManager
{
public:
	materialInstanceManager(graphics::Renderer& _renderer , core::gpu::Device& _device);
	~materialInstanceManager();

	std::shared_ptr<graphics::resources::MaterialInstance> CreateMaterialInstance(graphics::assets::Material& _material);

	void UpdateMaterialInstancesOf(std::string _MaterialName); //To call when we'll edit Materials;

private:
	const core::gpu::DescriptorSetLayout* GetMaterialDescriptorSetLayout(graphics::MaterialType _type);

private:
	std::unordered_map<std::string, std::vector<std::shared_ptr<graphics::resources::MaterialInstance>>> m_materialInstanceMap;

	core::gpu::Device& m_device;
	graphics::Renderer& m_renderer;
};