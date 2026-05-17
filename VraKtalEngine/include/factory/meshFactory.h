#pragma once
#include <factory/assetFactory.h>

namespace graphics {
	namespace assets {
		struct Mesh;
	}
	namespace resources{
		struct Mesh;
	}
}

namespace core::gpu {
	class Device;
}


namespace factory {
	
	struct MeshFactory //Factories are stored in ressourceManager
	{
		MeshFactory(core::gpu::Device& _device);
		graphics::resources::Mesh Create(const graphics::assets::Mesh& asset);
	
		static graphics::resources::Mesh CreateBuffersForMesh(const graphics::assets::Mesh& _mesh , core::gpu::Device* _device);
		static void CopyMeshMetadata(graphics::resources::Mesh& mesh, const graphics::assets::Mesh& asset);
		static void CreateBLASForMesh(graphics::resources::Mesh& mesh, const graphics::assets::Mesh& meshAsset, core::gpu::Device* _device);
		private:
		core::gpu::Device& m_device;
	};

}