#ifndef VRAKTAL_CORE_VKTYPES_H
#define VRAKTAL_CORE_VKTYPES_H
#pragma once 

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace vkTypes
{
    struct AllocatedImage
    {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkExtent3D imageExtent;
        VkFormat imageFormat;
    };

}

#endif //VRAKTAL_CORE_VKTYPES_H