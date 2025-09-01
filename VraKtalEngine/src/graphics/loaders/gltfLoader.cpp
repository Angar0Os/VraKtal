#include "gltfLoader.h"

#include <tiny_gltf.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

using namespace graphics::scene;
using namespace graphics::resources;

Scene LoadScene(const std::string& path)
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err, warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty())
    {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "Error: " << err << std::endl;
    }

    if (!ret)
    {
        throw std::runtime_error("Failed to load scene" + path);
    }

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
        scene.materials.push_back(material);
    }

    for (auto& mesh : model.meshes)
    {
        for (auto& prim : mesh.primitives)
        {
            Mesh mesh;

            const tinygltf::Accessor& posAccessor = model.accessors[prim.attributes.find("POSITION")->second];
            const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];
            const float* pos = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
            for (size_t i = 0; i < posAccessor.count; ++i)
            {
                Vertex vertex{};
                vertex.position = glm::make_vec3(&pos[i * 3]);
                mesh.vertices.push_back(vertex);
            }

            const tinygltf::Accessor& idxAccessor = model.accessors[prim.indices];
            const tinygltf::BufferView& idxBufferView = model.bufferViews[idxAccessor.bufferView];
            const tinygltf::Buffer& idxBuffer = model.buffers[idxBufferView.buffer];
            const void* dataPtr = &idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset];

            if (idxAccessor.count == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                for (size_t i = 0; i < idxAccessor.count; ++i)
                {
                    mesh.indices.push_back(buf[i]);
                }
            }
            if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
            {
                const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                for (size_t i = 0; i < idxAccessor.count; ++i)
                {
                    mesh.indices.push_back(buf[i]);
                }
            }

            mesh.materialIndex = prim.material;
            scene.meshes.push_back(mesh);
        }
    }

    return scene;
}