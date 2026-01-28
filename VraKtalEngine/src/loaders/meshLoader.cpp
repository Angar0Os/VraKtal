#include <loaders/meshLoader.h>
#include <core/gpu/commandBuffer.h>
#include <core/gpu/accelerationStructure.h>

#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

loaders::MeshLoader::MeshLoader(core::gpu::Device* device)
	: m_device(device)
{
}

void loaders::MeshLoader::CreateBuffersForMesh(graphics::resources::Mesh* mesh)
{
	if (!mesh || mesh->vertices.empty() || mesh->indices.empty())
	{
		std::cerr << "ERROR: Invalid mesh data for buffer creation!" << std::endl;
		return;
	}

	size_t vertexBufferSize = mesh->vertices.size() * sizeof(graphics::resources::Vertex);
	size_t indexBufferSize = mesh->indices.size() * sizeof(uint32_t);

	core::gpu::SBufferCreateInfo stagingVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	auto stagingVertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		stagingVertexInfo
	);

	core::gpu::SBufferCreateInfo stagingIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::TransferSrc,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	auto stagingIndexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		stagingIndexInfo
	);

	stagingVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);
	stagingIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

	core::gpu::SBufferCreateInfo vertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::VertexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};

	mesh->vertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		vertexInfo
	);

	core::gpu::SBufferCreateInfo indexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::IndexBuffer | core::EBufferUsage::TransferDst,
		.memoryProperties = core::EMemoryProperty::DeviceLocal
	};

	mesh->indexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		indexInfo
	);

	core::gpu::SBufferCreateInfo rtVertexInfo{
		.size = vertexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
				 core::EBufferUsage::ShaderDeviceAddress |
				 core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	mesh->rtVertexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		rtVertexInfo
	);
	mesh->rtVertexBuffer->CopyFrom(mesh->vertices.data(), vertexBufferSize);

	core::gpu::SBufferCreateInfo rtIndexInfo{
		.size = indexBufferSize,
		.usage = core::EBufferUsage::AccelerationStructureBuildInput |
				 core::EBufferUsage::ShaderDeviceAddress |
				 core::EBufferUsage::StorageBuffer,
		.memoryProperties = core::EMemoryProperty::HostVisible | core::EMemoryProperty::HostCoherent
	};

	mesh->rtIndexBuffer = std::make_unique<core::gpu::Buffer>(
		m_device,
		rtIndexInfo
	);
	mesh->rtIndexBuffer->CopyFrom(mesh->indices.data(), indexBufferSize);

	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = m_device;
	cmdInfo.level = core::ECommandBufferLevel::Primary;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;

	auto transferCmd = std::make_unique<core::gpu::CommandBuffer>(
		m_device,
		cmdInfo
	);

	transferCmd->Begin(0);
	transferCmd->CopyBuffer(
		stagingVertexBuffer.get(),
		mesh->vertexBuffer.get(),
		vertexBufferSize
	);

	transferCmd->CopyBuffer(
		stagingIndexBuffer.get(),
		mesh->indexBuffer.get(),
		indexBufferSize
	);
	transferCmd->End(0);

	transferCmd->SubmitAndWait(m_device);

	mesh->indexCount = static_cast<uint32_t>(mesh->indices.size());
}

void loaders::MeshLoader::CreateBLASForMesh(graphics::resources::Mesh* mesh)
{
	if (!mesh || !mesh->rtVertexBuffer || !mesh->rtIndexBuffer)
	{
		std::cerr << "ERROR: RT buffers not found for BLAS creation!" << std::endl;
		return;
	}

	core::gpu::SAccelerationStructureGeometry geometry{};
	geometry.vertexBuffer = mesh->rtVertexBuffer.get();
	geometry.vertexCount = static_cast<uint32_t>(mesh->vertices.size());
	geometry.vertexStride = sizeof(graphics::resources::Vertex);
	geometry.indexBuffer = mesh->rtIndexBuffer.get();
	geometry.indexCount = static_cast<uint32_t>(mesh->indices.size());
	geometry.triangleCount = geometry.indexCount / 3;
	geometry.opaque = true;

	core::gpu::SAccelerationStructureCreateInfo blasInfo{};
	blasInfo.type = core::gpu::EAccelerationStructureType::BottomLevel;
	blasInfo.geometries.push_back(geometry);
	blasInfo.preferFastTrace = true;
	blasInfo.allowUpdate = false;

	mesh->blas = std::make_unique<core::gpu::AccelerationStructure>(
		m_device,
		blasInfo
	);

	core::gpu::SCommandBufferCreateInfo cmdInfo{};
	cmdInfo.device = m_device;
	cmdInfo.count = 1;
	cmdInfo.singleTime = true;
	cmdInfo.level = core::ECommandBufferLevel::Primary;

	core::gpu::CommandBuffer cmdBuffer(
		m_device,
		cmdInfo
	);

	cmdBuffer.Begin(0);
	cmdBuffer.BuildAccelerationStructure(mesh->blas.get());
	cmdBuffer.End(0);
	cmdBuffer.SubmitAndWait(m_device);

	std::cout << "BLAS built for mesh during loading" << std::endl;
}

std::shared_ptr<graphics::resources::Mesh> loaders::MeshLoader::LoadMesh(const std::string& filepath)
{
	std::string ext = std::filesystem::path(filepath).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	std::shared_ptr<graphics::resources::Mesh> mesh;

	if (ext == ".gltf" || ext == ".glb")
	{
		mesh = LoadGLTF(filepath);
	}
	else if (ext == ".obj")
	{
		mesh = LoadOBJ(filepath);
	}
	else
	{
		throw std::runtime_error("Unsupported mesh format : " + ext);
	}

	if (mesh && m_device)
	{
		CreateBuffersForMesh(mesh.get());
		CreateBLASForMesh(mesh.get());
	}

	return mesh;
}

std::shared_ptr<graphics::resources::Mesh> loaders::MeshLoader::LoadGLTF(const std::string& filepath)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;
	bool ret = false;

	if (filepath.ends_with(".gltf"))
	{
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
	}
	else if (filepath.ends_with(".glb"))
	{
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
	}
	else
	{
		throw std::runtime_error("Unsupported GLTF extension: " + filepath);
	}

	if (!warn.empty()) std::cerr << "GLTF Warning: " << warn << "\n";
	if (!err.empty())  std::cerr << "GLTF Error: " << err << "\n";
	if (!ret) throw std::runtime_error("Failed to load GLTF: " + filepath);

	auto mesh = std::make_shared<graphics::resources::Mesh>();
	std::unordered_map<graphics::resources::Vertex, uint32_t> uniqueVertices{};

	if (model.meshes.empty())
	{
		throw std::runtime_error("GLTF contains no meshes");
	}

	const tinygltf::Mesh& gltfMesh = model.meshes[0];
	if (gltfMesh.primitives.empty())
	{
		throw std::runtime_error("GLTF mesh contains no primitives");
	}

	std::cout << "Loading GLTF mesh '" << gltfMesh.name
		<< "' with " << gltfMesh.primitives.size() << " primitives\n";

	for (size_t primIdx = 0; primIdx < gltfMesh.primitives.size(); ++primIdx)
	{
		const tinygltf::Primitive& prim = gltfMesh.primitives[primIdx];

		uint32_t submeshFirstIndex = static_cast<uint32_t>(mesh->indices.size());
		uint32_t submeshVertexOffset = static_cast<uint32_t>(mesh->vertices.size());

		auto getFloatVec = [&](const std::string& name, int elementSize) -> std::vector<float>
			{
				auto attrIt = prim.attributes.find(name);
				if (attrIt == prim.attributes.end())
					return {};

				const tinygltf::Accessor& accessor = model.accessors[attrIt->second];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				const float* ptr = reinterpret_cast<const float*>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
				return std::vector<float>(ptr, ptr + accessor.count * elementSize);
			};

		std::vector<float> positions = getFloatVec("POSITION", 3);
		std::vector<float> normals = getFloatVec("NORMAL", 3);
		std::vector<float> uvs = getFloatVec("TEXCOORD_0", 2);
		std::vector<float> tangents = getFloatVec("TANGENT", 4);

		std::vector<uint32_t> primitiveIndices;
		if (prim.indices >= 0)
		{
			const tinygltf::Accessor& accessor = model.accessors[prim.indices];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			primitiveIndices.resize(accessor.count);

			if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				const uint16_t* src = reinterpret_cast<const uint16_t*>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
				for (size_t i = 0; i < accessor.count; i++)
					primitiveIndices[i] = static_cast<uint32_t>(src[i]);
			}
			else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			{
				const uint32_t* src = reinterpret_cast<const uint32_t*>(
					&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
				for (size_t i = 0; i < accessor.count; i++)
					primitiveIndices[i] = src[i];
			}
		}

		std::unordered_map<graphics::resources::Vertex, uint32_t> localVertexMap;
		uint32_t localVertexCount = 0;

		for (size_t i = 0; i < positions.size() / 3; i++)
		{
			graphics::resources::Vertex v{};
			v.position = { positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] };
			if (!normals.empty())
				v.normal = { normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] };
			if (!uvs.empty())
				v.uv = { uvs[i * 2 + 0], uvs[i * 2 + 1] };
			if (!tangents.empty())
				v.tangent = { tangents[i * 4 + 0], tangents[i * 4 + 1],
							  tangents[i * 4 + 2], tangents[i * 4 + 3] };
			else
				v.tangent = { 1.0f, 1.0f, 1.0f, 1.0f };

			if (!uniqueVertices.contains(v))
			{
				uniqueVertices[v] = static_cast<uint32_t>(mesh->vertices.size());
				mesh->vertices.push_back(v);
			}

			localVertexMap[v] = uniqueVertices[v];
			localVertexCount++;
		}

		if (!primitiveIndices.empty())
		{
			for (auto idx : primitiveIndices)
			{
				graphics::resources::Vertex originalVertex{};
				originalVertex.position = {
					positions[idx * 3 + 0],
					positions[idx * 3 + 1],
					positions[idx * 3 + 2]
				};
				if (!normals.empty())
					originalVertex.normal = {
						normals[idx * 3 + 0],
						normals[idx * 3 + 1],
						normals[idx * 3 + 2]
				};
				if (!uvs.empty())
					originalVertex.uv = { uvs[idx * 2 + 0], uvs[idx * 2 + 1] };
				if (!tangents.empty())
					originalVertex.tangent = {
						tangents[idx * 4 + 0], tangents[idx * 4 + 1],
						tangents[idx * 4 + 2], tangents[idx * 4 + 3]
				};
				else
					originalVertex.tangent = { 1.0f, 1.0f, 1.0f, 1.0f };

				mesh->indices.push_back(uniqueVertices[originalVertex]);
			}
		}

		graphics::resources::SubMesh submesh;
		submesh.firstIndex = submeshFirstIndex;
		submesh.indexCount = static_cast<uint32_t>(mesh->indices.size() - submeshFirstIndex);
		submesh.vertexOffset = submeshVertexOffset;
		submesh.materialIndex = (prim.material >= 0) ? prim.material : 0;
		submesh.name = "primitive_" + std::to_string(primIdx);

		mesh->subMeshes.push_back(submesh);
	}

	return mesh;
}

std::shared_ptr<graphics::resources::Mesh> loaders::MeshLoader::LoadOBJ(const std::string& filepath)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

	auto mesh = std::make_shared<graphics::resources::Mesh>();
	std::unordered_map<graphics::resources::Vertex, uint32_t> uniqueVertices{};

	for (size_t shapeIdx = 0; shapeIdx < shapes.size(); ++shapeIdx)
	{
		const auto& shape = shapes[shapeIdx];

		uint32_t submeshFirstIndex = static_cast<uint32_t>(mesh->indices.size());
		uint32_t submeshVertexOffset = static_cast<uint32_t>(mesh->vertices.size());

		for (const auto& index : shape.mesh.indices)
		{
			graphics::resources::Vertex vertex{};

			if (index.vertex_index >= 0)
			{
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
			}

			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}
			else
			{
				vertex.uv = { 0.0f, 0.0f };
			}

			if (index.normal_index >= 0)
			{
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}
			else
			{
				vertex.normal = { 0.0f, 1.0f, 0.0f };
			}

			vertex.tangent = { 1.0f, 0.0f, 0.0f, 1.0f };

			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh->vertices.size());
				mesh->vertices.push_back(vertex);
			}

			mesh->indices.push_back(uniqueVertices[vertex]);
		}

		graphics::resources::SubMesh submesh;
		submesh.firstIndex = submeshFirstIndex;
		submesh.indexCount = static_cast<uint32_t>(mesh->indices.size() - submeshFirstIndex);
		submesh.vertexOffset = submeshVertexOffset;
		submesh.materialIndex = 0;
		submesh.name = shape.name.empty() ? ("shape_" + std::to_string(shapeIdx)) : shape.name;

		mesh->subMeshes.push_back(submesh);
	}

	return mesh;
}

std::shared_ptr<graphics::resources::Mesh> loaders::MeshLoader::CreatePlane(
	float width, float height, int subdivisionsX, int subdivisionsZ)
{
	auto mesh = std::make_shared<graphics::resources::Mesh>();

	int vertCountX = subdivisionsX + 1;
	int vertCountZ = subdivisionsZ + 1;

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;

	float deltaX = width / subdivisionsX;
	float deltaZ = height / subdivisionsZ;

	float uvDeltaX = 1.0f / subdivisionsX;
	float uvDeltaZ = 1.0f / subdivisionsZ;

	for (int z = 0; z < vertCountZ; z++)
	{
		for (int x = 0; x < vertCountX; x++)
		{
			graphics::resources::Vertex vertex;

			vertex.position = glm::vec3(
				-halfWidth + x * deltaX,
				0.0f,
				-halfHeight + z * deltaZ
			);

			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vertex.uv = glm::vec2(x * uvDeltaX, z * uvDeltaZ);

			vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

			mesh->vertices.push_back(vertex);
		}
	}

	for (int z = 0; z < subdivisionsZ; z++)
	{
		for (int x = 0; x < subdivisionsX; x++)
		{
			int topLeft = z * vertCountX + x;
			int topRight = topLeft + 1;
			int bottomLeft = (z + 1) * vertCountX + x;
			int bottomRight = bottomLeft + 1;

			mesh->indices.push_back(topLeft);
			mesh->indices.push_back(bottomLeft);
			mesh->indices.push_back(topRight);

			mesh->indices.push_back(topRight);
			mesh->indices.push_back(bottomLeft);
			mesh->indices.push_back(bottomRight);
		}
	}

	if (m_device)
	{
		CreateBuffersForMesh(mesh.get());
		CreateBLASForMesh(mesh.get());
	}

	return mesh;
}