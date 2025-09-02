#include "gltfLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <iostream>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <cfloat>

using namespace graphics::scene;
using namespace graphics::resources;

Scene graphics::loaders::LoadScene(const std::string& path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ret = false;

    if (path.ends_with(".gltf"))
    {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }
    else if (path.ends_with(".glb"))
    {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    }
    else
    {
        throw std::runtime_error("Unsupported file extention " + path);
    }

    if (!warn.empty())
        std::cout << "Warn: " << warn << std::endl;
    if (!err.empty())
        std::cout << "Error: " << err << std::endl;
    if (!ret)
        throw std::runtime_error("Failed to load scene " + path);

    Scene scene;
    
    for (auto& mat : model.materials)
    {
        Material material;
        if (mat.values.find("baseColorFactor") != mat.values.end())
        {
            auto factor = mat.values.at("baseColorFactor").ColorFactor();
            material.baseColorFactor = glm::vec4(factor[0], factor[1], factor[2], factor[3]);
        }
        if (mat.values.find("metallicFactor") != mat.values.end())
        {
            material.matallicFactor = (float)mat.values.at("metallicFactor").Factor();
        }
        if (mat.values.find("roughnessFactor") != mat.values.end())
        {
            material.roughnessFactor = (float)mat.values.at("roughnessFactor").Factor();
        }

        if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0)
        {
			int texIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
            if (texIndex < (int)model.textures.size())
            {
				int imgIdx = model.textures[texIndex].source;
                material.baseColorTexture = imgIdx;
            }
        }
        else
        {
            material.baseColorTexture = -1;
        }

        scene.materials.push_back(material);
    }

    for (auto& gltfMesh : model.meshes)
    {
        for (auto& prim : gltfMesh.primitives)
        {
            Mesh mesh;

            const tinygltf::Accessor& posAccessor = model.accessors[prim.attributes.find("POSITION")->second];
            const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];
            const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

            const float* normals = nullptr;
            if (prim.attributes.find("NORMAL") != prim.attributes.end())
            {
                const tinygltf::Accessor& normAccessor = model.accessors[prim.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& normBufferView = model.bufferViews[normAccessor.bufferView];
                const tinygltf::Buffer& normBuffer = model.buffers[normBufferView.buffer];
                normals = reinterpret_cast<const float*>(&normBuffer.data[normBufferView.byteOffset + normAccessor.byteOffset]);
            }

            const float* uvs = nullptr;
            if (prim.attributes.find("TEXCOORD_0") != prim.attributes.end())
            {
                const tinygltf::Accessor& uvAccessor = model.accessors[prim.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& uvBufferView = model.bufferViews[uvAccessor.bufferView];
                const tinygltf::Buffer& uvBuffer = model.buffers[uvBufferView.buffer];
                uvs = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
            }

            for (size_t i = 0; i < posAccessor.count; ++i)
            {
                Vertex vertex{};
                vertex.position = glm::make_vec3(&positions[i * 3]);
                vertex.normal = normals ? glm::make_vec3(&normals[i * 3]) : glm::vec3(0.0f, 1.0f, 0.0f);
                vertex.uv = uvs ? glm::make_vec2(&uvs[i * 2]) : glm::vec2(0.0f, 0.0f);
                vertex.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

                mesh.vertices.push_back(vertex);
            }

            const tinygltf::Accessor& idxAccessor = model.accessors[prim.indices];
            const tinygltf::BufferView& idxBufferView = model.bufferViews[idxAccessor.bufferView];
            const tinygltf::Buffer& idxBuffer = model.buffers[idxBufferView.buffer];
            const void* dataPtr = &idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset];

            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                for (size_t i = 0; i < idxAccessor.count; ++i)
                    mesh.indices.push_back(buf[i]);
            }
            else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                for (size_t i = 0; i < idxAccessor.count; ++i)
                    mesh.indices.push_back(buf[i]);
            }

            mesh.materialIndex = prim.material;

            if ((int)mesh.materialIndex < 0)
            {
                mesh.materialIndex = 0;
            }

            scene.meshes.push_back(mesh);
        }
    }

    std::filesystem::path baseDir = std::filesystem::path(path).parent_path();
    for (auto& img : model.images)
    {
        std::string texPath;
        if (!img.uri.empty())
        {
            std::filesystem::path texturePath = baseDir / img.uri;
            texPath = texturePath.lexically_normal().string();
        }
        else if (!img.image.empty())
        {
            texPath = "<embedded>"; 
        }
        else
        {
            texPath = "<invalid>";
        }

        scene.textures.push_back(texPath);
    }

    if (model.cameras.empty())
    {
        glm::vec3 min(FLT_MAX), max(-FLT_MAX);
        for (auto& mesh : scene.meshes)
        {
            for (auto& v : mesh.vertices)
            {
                min = glm::min(min, v.position);
                max = glm::max(max, v.position);
            }
        }
        glm::vec3 center = (min + max) * 0.5f;
        float radius = glm::length(max - min) * 0.5f;

        scene.defaultCamera.view = glm::lookAt(center + glm::vec3(0, 0, radius * 2.0f),
            center,
            glm::vec3(0, 1, 0));
        scene.defaultCamera.proj = glm::perspective(glm::radians(60.0f),
            16.0f / 9.0f,
            0.1f,
            radius * 10.0f);
    }

    return scene;
}
