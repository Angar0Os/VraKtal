#include <vk/instance.h>

#include <GLFW/glfw3.h>

using namespace vk;

void Instance::init(VkInstance _instance, GLFWwindow* _window, VkAllocationCallbacks* _callbacks, VkSurfaceKHR _surface,
	VkDebugUtilsMessengerEXT _debug_messenger, bool useValidationLayers)
{
	vkb::InstanceBuilder builder;

	auto inst_ret = builder.set_app_name("HelloTriangleApplication")
		.request_validation_layers(useValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	_instance = vkb_inst.instance;
	_debug_messenger = vkb_inst.debug_messenger;
	_callbacks = vkb_inst.allocation_callbacks;


	glfwCreateWindowSurface(_instance, _window, _callbacks, &_surface);
}