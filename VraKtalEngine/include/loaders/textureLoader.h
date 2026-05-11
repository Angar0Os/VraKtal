#pragma once
#include <loaders/loaderBase.h>


namespace core::gpu {

	class Device;
}

namespace loaders {
	struct TextureLoadOptions
	{
		bool isSRGB = false;
	};
	
	class TextureLoader : public LoaderBase
	{
	public:
		TextureLoader( core::gpu::Device* _device);
		virtual ~TextureLoader() override;
	
		std::shared_ptr<void> Load(const std::string& path, const LoadOptions* options) override;
	
	private:
		core::gpu::Device& m_device;
	};
}