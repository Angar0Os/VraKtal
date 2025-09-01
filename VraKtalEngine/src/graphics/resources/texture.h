#ifndef VRAKTAL_GRAPHICS_RESOURCES_TEXTURE_H
#define VRAKTAL_GRAPHICS_RESOURCES_TEXTURE_H
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <string>

namespace graphics::resources
{
	struct TextureGpu
	{
        VkImage        image        = VK_NULL_HANDLE;
        VmaAllocation  allocation   = VK_NULL_HANDLE;
        VkImageView    view         = VK_NULL_HANDLE;
        VkSampler      sampler      = VK_NULL_HANDLE;
        uint32_t       width        = 0;
        uint32_t       height       = 0;
        bool           ownsImage    = true;
	};
}

#endif //VRAKTAL_GRAPHICS_RESOURCES_TEXTURE_H
