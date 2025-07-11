#ifndef VRAKTAL_VK_INSTANCE_H
#define VRAKTAL_VK_INSTANCE_H
#pragma once

#include "../src/vkb/VkBootstrap.h"

struct GLFWwindow;

namespace vk
{
	class Instance
	{
	public:
		Instance() = default;
		~Instance() noexcept = default;

		void init(VkInstance _instance, GLFWwindow* _window, VkAllocationCallbacks* _callbacks, VkSurfaceKHR _surface,
			VkDebugUtilsMessengerEXT _debug_messenger, bool useValidationLayers);
	};
}

#endif // VRAKTAL_VK_INSTANCE_H
