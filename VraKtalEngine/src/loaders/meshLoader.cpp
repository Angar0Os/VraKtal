#include <loaders/meshLoader.h>

#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

std::shared_ptr<graphics::resources::Mesh> loaders::MeshLoader::LoadMesh(const std::string& filepath)
{
	std::string ext = std::filesystem::path(filepath).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	if (ext == ".gltf" || ext == ".glb")
	{
		return LoadGLTF(filepath);
	}
	else if (ext == ".obj")
	{
		return LoadOBJ(filepath);
	}
	else
	{
		throw std::runtime_error("Unsupported mesh format : " + ext);
	}
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

	const tinygltf::Primitive& prim = gltfMesh.primitives[0];

	auto getFloatVec = [&](const std::string& name, int elementSize) -> std::vector<float>
		{
			auto attrIt = prim.attributes.find(name);
			if (attrIt == prim.attributes.end())
				return {};

			const tinygltf::Accessor& accessor = model.accessors[attrIt->second];
			const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

			const float* ptr = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
			return std::vector<float>(ptr, ptr + accessor.count * elementSize);
		};

	std::vector<float> positions = getFloatVec("POSITION", 3);
	std::vector<float> normals = getFloatVec("NORMAL", 3);
	std::vector<float> uvs = getFloatVec("TEXCOORD_0", 2);
	std::vector<float> tangents = getFloatVec("TANGENT", 4);

	std::vector<uint32_t> indices;
	if (prim.indices >= 0)
	{
		const tinygltf::Accessor& accessor = model.accessors[prim.indices];
		const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

		indices.resize(accessor.count);

		if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
		{
			const uint16_t* src = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
			for (size_t i = 0; i < accessor.count; i++) indices[i] = static_cast<uint32_t>(src[i]);
		}
		else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		{
			const uint32_t* src = reinterpret_cast<const uint32_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
			for (size_t i = 0; i < accessor.count; i++) indices[i] = src[i];
		}
		else
		{
			throw std::runtime_error("Unsupported GLTF index component type");
		}
	}

	for (size_t i = 0; i < positions.size() / 3; i++)
	{
		graphics::resources::Vertex v{};
		v.position = { positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] };
		if (!normals.empty()) v.normal = { normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] };
		if (!uvs.empty())     v.uv = { uvs[i * 2 + 0], uvs[i * 2 + 1] };
		if (!tangents.empty()) v.tangent = { tangents[i * 4 + 0], tangents[i * 4 + 1], tangents[i * 4 + 2], tangents[i * 4 + 3] };
		else v.tangent = { 1.0f,1.0f,1.0f,1.0f };

		if (!uniqueVertices.contains(v))
		{
			uniqueVertices[v] = static_cast<uint32_t>(mesh->vertices.size());
			mesh->vertices.push_back(v);
		}
	}

	if (!indices.empty())
	{
		for (auto idx : indices)
		{
			mesh->indices.push_back(idx);
		}
	}
	else
	{
		for (uint32_t i = 0; i < mesh->vertices.size(); i++)
		{
			mesh->indices.push_back(i);
		}
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

	for (const auto& shape : shapes)
	{
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
	}

	return mesh;
}