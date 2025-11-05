#ifndef VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#define VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H
#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <core/enum.h>

namespace core::gpu_detail
{
	vk::ImageLayout ToVulkan(ImageLayout layout);
	vk::Filter		ToVulkan(Filter filter);
}

#endif //VRAKTAL_CORE_GPU_DETAIL_CONVERTERS_H