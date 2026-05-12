#include <factory/meshFactory.h>
#include <graphics/resources/object/mesh.h>
#include <graphics/assets/mesh.h>

#include <iostream>
#include <memory>
#include <core/gpu/commandBuffer.h>

using namespace factory;
factory::MeshFactory::MeshFactory(core::gpu::Device& _device) : m_device(_device){}

graphics::resources::Mesh MeshFactory::Create(const graphics::assets::Mesh& asset)
{
	graphics::resources::Mesh mesh = CreateBuffersForMesh(asset, &m_device);
    EnsureDefaultSubmesh(mesh, const_cast<graphics::assets::Mesh&>(asset)); // EnsureDefaultSubmesh modifies the asset's submeshes if needed
	CreateBLASForMesh(mesh, asset, &m_device);
	return mesh;
}


graphics::resources::Mesh MeshFactory::CreateBuffersForMesh(const graphics::assets::Mesh& _meshAsset , core::gpu::Device* _device)
{
	if (_meshAsset.vertices.empty() || _meshAsset.indices.empty())
	{
		std::cerr << "ERROR: Invalid _meshAsset data for buffer creation!" << std::endl;
	}

	size_t vertexBufferSize = _meshAsset.vertices.size() * sizeof(graphics::Vertex);
	size_t indexBufferSize = _meshAsset.indices.size() * sizeof(uint32_t);

	graphics::resources::Mesh mesh;

	core::gpu::SBufferCreateInfo stagingVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};
	auto stagingVertexBuffer = std::make_unique<core::gpu::Buffer>(_device, stagingVertexInfo);

	core::gpu::SBufferCreateInfo stagingIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};
	auto stagingIndexBuffer = std::make_unique<core::gpu::Buffer>(_device, stagingIndexInfo);

	stagingVertexBuffer->CopyFrom(_meshAsset.vertices.data(), vertexBufferSize);
	stagingIndexBuffer->CopyFrom(_meshAsset.indices.data(), indexBufferSize);

	core::gpu::SBufferCreateInfo vertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::VertexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};
	mesh.vertexBuffer = std::make_unique<core::gpu::Buffer>(_device, vertexInfo);

	core::gpu::SBufferCreateInfo indexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::IndexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};
	mesh.indexBuffer = std::make_unique<core::gpu::Buffer>(_device, indexInfo);

	core::gpu::SBufferCreateInfo rtVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
							core::EBufferUsage::ShaderDeviceAddress |
							core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};
	mesh.rtVertexBuffer = std::make_unique<core::gpu::Buffer>(_device, rtVertexInfo);
	mesh.rtVertexBuffer->CopyFrom(_meshAsset.vertices.data(), vertexBufferSize);

	core::gpu::SBufferCreateInfo rtIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
							core::EBufferUsage::ShaderDeviceAddress |
							core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};
	mesh.rtIndexBuffer = std::make_unique<core::gpu::Buffer>(_device, rtIndexInfo);
	mesh.rtIndexBuffer->CopyFrom(_meshAsset.indices.data(), indexBufferSize);

	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = _device;
	cmdInfo.level = core::ECommandBufferLevel::Primary;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;

	auto transferCmd = std::make_unique<core::gpu::CommandBuffer>(_device, cmdInfo);

	transferCmd->Begin(0);
	transferCmd->CopyBuffer(stagingVertexBuffer.get(), mesh.vertexBuffer.get(), vertexBufferSize);
	transferCmd->CopyBuffer(stagingIndexBuffer.get(), mesh.indexBuffer.get(), indexBufferSize);
	transferCmd->End(0);
	transferCmd->SubmitImmediate(_device);

	//_meshAsset.indexCount = static_cast<uint32_t>(_meshAsset.indices.size());

	return mesh;
}

void factory::MeshFactory::CreateBLASForMesh(graphics::resources::Mesh& mesh, const graphics::assets::Mesh& meshAsset , core::gpu::Device* _device)
{
	if (!mesh.rtVertexBuffer || !mesh.rtIndexBuffer)
	{
		std::cerr << "ERROR: RT buffers not found for BLAS creation!" << std::endl;
		return;
	}

	core::gpu::SAccelerationStructureGeometry geometry{};
	geometry.vertexBuffer = mesh.rtVertexBuffer.get();
	geometry.vertexCount = static_cast<uint32_t>(meshAsset.vertices.size());
	geometry.vertexStride = sizeof(graphics::Vertex);
	geometry.indexBuffer = mesh.rtIndexBuffer.get();
	geometry.indexCount = static_cast<uint32_t>(meshAsset.indices.size());
	geometry.triangleCount = geometry.indexCount / 3;
	geometry.opaque = true;

	core::gpu::SAccelerationStructureCreateInfo blasInfo{};
	blasInfo.type = core::gpu::EAccelerationStructureType::BottomLevel;
	blasInfo.geometries.push_back(geometry);
	blasInfo.preferFastTrace = true;
	blasInfo.allowUpdate = false;

	mesh.blas = std::make_unique<core::gpu::AccelerationStructure>(_device, blasInfo);

	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = _device;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = core::ECommandBufferLevel::Primary;

	core::gpu::CommandBuffer cmdBuffer(_device, cmdInfo);

	cmdBuffer.Begin(0);
	cmdBuffer.BuildAccelerationStructure(mesh.blas.get());
	cmdBuffer.End(0);
	cmdBuffer.SubmitImmediate(_device);

	std::cout << "BLAS built for mesh during loading" << std::endl;
}

void factory::MeshFactory::EnsureDefaultSubmesh(graphics::resources::Mesh& _mesh, graphics::assets::Mesh& _meshAsset)
{
	if (!_mesh.subMeshes.empty())
		return;

	graphics::SubMesh defaultSubmesh;
	defaultSubmesh.firstIndex = 0;
	defaultSubmesh.indexCount = static_cast<uint32_t>(_meshAsset.indices.size());
	defaultSubmesh.vertexOffset = 0;
	defaultSubmesh.materialIndex = 0;
	defaultSubmesh.name = "default";

	_mesh.subMeshes.push_back(defaultSubmesh);
}
