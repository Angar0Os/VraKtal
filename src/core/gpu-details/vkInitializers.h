#ifndef VRAKTAL_CORE_GPUDETAILS_VKINITIALIZER_H_
#define VRAKTAL_CORE_GPUDETAILS_VKINITIALIZER_H_
#pragma once

#include <vulkan/vulkan.h>

namespace core::gpu_details
{
    VkImageCreateInfo               ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo           ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
}

#endif //VRAKTAL_CORE_GPUDETAILS_VKINITIALIZER_H_