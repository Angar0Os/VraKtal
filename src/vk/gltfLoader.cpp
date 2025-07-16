#include <vk/gltfLoader.h>
#include <vk/image.h>
#include <vk/device.h>
#include <vk/buffer.h>
#include <vk/commandBuffer.h>
#include <vkEngine.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#ifndef NDEBUG
# pragma comment(lib, "fastgltf_simdjsond.lib")
# pragma comment(lib, "fastgltfd.lib")
#else
# pragma comment(lib, "fastgltf_simdjson.lib")
# pragma comment(lib, "fastgltf.lib")
#endif /* !NDEBUG */

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/parser.hpp>

LoadedGLTF::LoadedGLTF(VulkanEngine* engine, VulkanDevice* device, VulkanBuffer* buffer, VulkanImage* image, DescriptorAllocator* descriptorPool)
{
    ClearAll(engine, device, buffer, image, descriptorPool);
}

std::optional<AllocatedImage> LoadImage(VulkanEngine* engine, VulkanImage* image, fastgltf::Asset& asset, fastgltf::Image& fgltf_image)
{
    AllocatedImage newImage{};

    int width, height, nrChannels;

    std::visit(
        fastgltf::visitor{
            [](auto& arg) {},
            [&](fastgltf::sources::URI& filePath) {
                assert(filePath.fileByteOffset == 0);
                assert(filePath.uri.isLocalPath());


                const std::string path(filePath.uri.path().begin(),
                    filePath.uri.path().end());
                unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
                if (data) {
                    VkExtent3D imagesize;
                    imagesize.width = width;
                    imagesize.height = height;
                    imagesize.depth = 1;

                    newImage = image->CreateImage(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,false);

                    stbi_image_free(data);
                }
                },
                [&](fastgltf::sources::Vector& vector) {
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data(), static_cast<int>(vector.bytes.size()),
                        &width, &height, &nrChannels, 4);
                    if (data) {
                        VkExtent3D imagesize;
                        imagesize.width = width;
                        imagesize.height = height;
                        imagesize.depth = 1;

                        newImage = image->CreateImage(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT,false);

                        stbi_image_free(data);
                    }
                },
                [&](fastgltf::sources::BufferView& view) {
                    auto& bufferView = asset.bufferViews[view.bufferViewIndex];
                    auto& buffer = asset.buffers[bufferView.bufferIndex];

                    std::visit(fastgltf::visitor {
                [](auto& arg) {},
                [&](fastgltf::sources::Vector& vector) {
                    unsigned char* data = stbi_load_from_memory(vector.bytes.data() + bufferView.byteOffset,
                        static_cast<int>(bufferView.byteLength),
                        &width, &height, &nrChannels, 4);
                    if (data) {
                        VkExtent3D imagesize;
                        imagesize.width = width;
                        imagesize.height = height;
                        imagesize.depth = 1;

                        newImage = image->CreateImage(data, imagesize, VK_FORMAT_R8G8B8A8_UNORM,
                            VK_IMAGE_USAGE_SAMPLED_BIT,false);

                        stbi_image_free(data);
                            }
                        } },
                buffer.data);
            },
        },
        fgltf_image.data);

    if (newImage.image == VK_NULL_HANDLE)
    {
        return {};
    }
    else
    {
        return newImage;
    }
}

VkFilter ExtractFilter(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return VK_FILTER_NEAREST;

    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode ExtractMipmapMode(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

// TODO : IMPLEMENT THIS
//std::optional<std::shared_ptr<LoadedGLTF>> LoadGLTF(VulkanEngine* engine, std::string_view filepath)
//{
//
//}

void LoadedGLTF::Draw(const glm::mat4& topMatrix, DrawContext& ctx)
{

}

void LoadedGLTF::ClearAll(VulkanEngine* engine, VulkanDevice* device, VulkanBuffer* buffer, VulkanImage* image, DescriptorAllocator* descriptorPool)
{
    VkDevice dv = device->GetVkDevice();

    for (auto& [k, v] : meshes)
    {

        buffer->Destroy(*v->meshBuffers.indexBuffer);
        buffer->Destroy(*v->meshBuffers.vertexBuffer);
    }

    for (auto& [k, v] : images)
    {

        if (v.image == engine->_errorCheckerboardImage->image)
        {
            continue;
        }
        image->DestroyImage(v);
    }

    for (auto& sampler : samplers)
    {
        vkDestroySampler(dv, sampler, nullptr);
    }

    auto materialBuffer = materialDataBuffer;
    auto samplersToDestroy = samplers;

    descriptorPool->DestroyPool(dv);

    buffer->Destroy(*materialBuffer);
}