#ifndef VRAKTAL_CORE_VKTYPES_H
#define VRAKTAL_CORE_VKTYPES_H
#pragma once 

#include <deque>
#include <functional>
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

    struct DeletionQueue
    {
        std::deque<std::function<void()>> deletors;

        void PushFunction(std::function<void()>&& function)
        {
            deletors.push_back(function);
        }

        void Flush()
        {
            for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
            {
                (*it)();
            }

            deletors.clear();
        }
    };

}

#endif //VRAKTAL_CORE_VKTYPES_H