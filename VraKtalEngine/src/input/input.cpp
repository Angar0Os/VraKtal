#include <core/input/input.h>

#include <core/window.h>
#include <core/gpu/device.h>
#include <core/gpu/imguiContext.h>

#include <iostream>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>


core::Input* core::Input::m_instance = nullptr;

struct core::Input::Internal
{
	GLFWwindow* window = nullptr;
};

core::Input::Input(core::Window& window ,core::gpu::Device* _device) : m_internal(new Internal)
{
	m_internal->window = static_cast<GLFWwindow*>(window.GlfwHandle());
	m_instance = this; // Set the static instance pointer to this instance

	CurrentKeys.resize(359);
	PreviousKeys.resize(359);

	glfwSetKeyCallback(m_internal->window, key_callback);
	glfwSetCursorPosCallback(m_internal->window, mouse_callback);
	glfwSetFramebufferSizeCallback(m_internal->window, framebuffer_size_callback);
	glfwSetScrollCallback(m_internal->window, scroll_callback);
	glfwSetMouseButtonCallback(m_internal->window, mouse_button_callback);

	m_device = _device;
}

core::Input::~Input()
{
	if (m_internal->window)
	{
		glfwSetKeyCallback(m_internal->window, nullptr);
		glfwSetCursorPosCallback(m_internal->window, nullptr);
		glfwSetFramebufferSizeCallback(m_internal->window, nullptr);
		glfwSetScrollCallback(m_internal->window, nullptr);
		m_internal->window = nullptr;
	}
}

core::Input* core::Input::GetInstance()
{
	if (m_instance == nullptr)
	{
		return nullptr;
	}
	return m_instance;
}

void core::Input::DestroyInstance()
{
	m_instance = nullptr;
}

input::KeyState core::Input::GetBindingState(const std::vector<input::Key>& keys) const
{
	if (keys.size() == 0)
		return input::KeyState::Up;
	 
	bool currentActive = true;
	 bool previousActive = true;

	 for (int key : keys)
	 {
		 currentActive &= CurrentKeys[key];
		 previousActive &= PreviousKeys[key];
	 }

	 if (currentActive)
		 return previousActive ? input::KeyState::OnGoing : input::KeyState::Press;

	 return previousActive ? input::KeyState::Release : input::KeyState::Up;
}

std::vector<int> core::Input::GetCurrentKeyPressed()
{
	std::vector<int> ActiveKeyIds;
	for (size_t i = 0; i < CurrentKeys.size(); i++)
	{
		if (CurrentKeys[i])
		{
			ActiveKeyIds.push_back(i);
		}
	}
	return ActiveKeyIds;
}

void core::Input::Update()
{
	// Update mouse movement
	float _xoffset = m_mousePosition.x - LastMousePosition.x;
	float _yoffset = LastMousePosition.y - m_mousePosition.y; // reversed since y-coordinates go from bottom to top

	MouseMovement = glm::vec2(_xoffset, _yoffset);
	LastMousePosition = m_mousePosition;

    m_mouseBind.Execute(MouseMovement);

	for (auto act : ActionsStorage.GetActiveValues())
	{
		input::KeyState bindingState = GetBindingState(act->keys);

		if (bindingState != input::KeyState::Up)
		{
			CallAction(act, bindingState);
		}
	}


	for (auto axisId : Axis2DActionsStorage.GetActiveValues())
	{
		glm::vec2 value(0.0f);

        value.x += CurrentKeys[static_cast<size_t>(axisId->key[0])] ? 1.0f : 0.0f;
        value.x -= CurrentKeys[static_cast<size_t>(axisId->key[1])] ? 1.0f : 0.0f;
        value.y += CurrentKeys[static_cast<size_t>(axisId->key[2])] ? 1.0f : 0.0f;
        value.y -= CurrentKeys[static_cast<size_t>(axisId->key[3])] ? 1.0f : 0.0f;
        axisId->FunctionAxis2D.Execute(value);
	}

	PreviousKeys = CurrentKeys;
}

void core::Input::CallAction(const KeyActions* _actions, input::KeyState actionType)
{
	switch (actionType)
	{
	case input::KeyState::Up: //Up
		break;
	case input::KeyState::Release: //Released
		for (const InputCallback& callback : _actions->Release)
		{
			callback.Execute();
		}
		break;
	case input::KeyState::Press: //Pressed
		for (const InputCallback& callback : _actions->Press)
		{
			callback.Execute();
		}
		break;
	case input::KeyState::OnGoing: //OnGoing
		for (const InputCallback& callback : _actions->OnGoing)
		{
			callback.Execute();
		}
		break;
	default:
		return;
		break;
	}
}

void core::Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) //static
{
	if (key == -1) // très drole mais fn sur mon clavier faisait crash l'appli
		return;

#ifdef VRAKTAL_EDITOR
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
#endif // VRAKTAL_EDITOR



	Input* ContextInput = Input::GetInstance();

	switch (action)
	{
	case 0: //Released
		ContextInput->CurrentKeys[key] = false;
		break;
	case 1: //Pressed
		ContextInput->CurrentKeys[key] = true;
		break;
	default:
		break;
	}
}

void core::Input::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
#ifdef VRAKTAL_EDITOR
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
#endif // VRAKTAL_EDITOR



	Input* ContextInput = Input::GetInstance();

	switch (action)
	{
	case 0: //Released
		ContextInput->CurrentKeys[button + 347] = false;
		break;
	case 1: //Pressed
		ContextInput->CurrentKeys[button + 347] = true;
		break;
	default:
		break;
	}
}

void core::Input::AddAction(std::string _actionName)
{
    ActionsStorage.Add(_actionName);
}

void core::Input::RemoveAction(std::string _actionName)
{
    ActionsStorage.Remove(_actionName);
}

void core::Input::AddAxis2DAction(std::string _actionName, input::Key _positiveX, input::Key _negativeX, input::Key _positiveY, input::Key _negativeY)
{
    Axis2DActionsStorage.Add(_actionName);
    Axis2DActionsStorage.Get(Axis2DActionsStorage.Find(_actionName)).key[0] = _positiveX;
    Axis2DActionsStorage.Get(Axis2DActionsStorage.Find(_actionName)).key[1] = _negativeX;
    Axis2DActionsStorage.Get(Axis2DActionsStorage.Find(_actionName)).key[2] = _positiveY;
    Axis2DActionsStorage.Get(Axis2DActionsStorage.Find(_actionName)).key[3] = _negativeY;
}

void core::Input::RemoveAxis2DAction(std::string _actionName)
{
    Axis2DActionsStorage.Remove(_actionName);
}

void core::Input::BindFunctionToAction(std::string _actionName, InputCallback _functionToCall, input::KeyState _onWhat)
{
	if (!ActionsStorage.Contains(_actionName))
		return;
	ActionID actionID = ActionsStorage.Find(_actionName);

	std::cout << "Trying to bind " << _actionName << " to " << input::KeyStateToString(_onWhat) << std::endl;

	switch (_onWhat)
	{
	case 0:
		break;
	case 1:
			ActionsStorage.Get(actionID).Release.push_back(_functionToCall);
			std::cout << "Binded " << _actionName << " to "<< input::KeyStateToString(_onWhat) << std::endl;
		break;
	case 2:
			ActionsStorage.Get(actionID).Press.push_back(_functionToCall);
			std::cout << "Binded " << _actionName << " to "<< input::KeyStateToString(_onWhat) << std::endl;
		break;
	case 3:
			ActionsStorage.Get(actionID).OnGoing.push_back(_functionToCall);
			std::cout << "Binded " << _actionName << " to "<< input::KeyStateToString(_onWhat) << std::endl;
		break;
	default:
		break;
	}
}

void core::Input::AddScrollAction(void* object, void(*func)(double, double))
{
	std::pair<void*, void(*)(double, double)> ToAdd(object, func);
	ScrollBind.emplace(ToAdd);
}

void core::Input::RemoveScrollAction(void* object, void(*func)(double, double))
{
	ScrollBind.erase(object);
}

void core::Input::mouse_callback(GLFWwindow* window, double xpos, double ypos) //static called from GLFWWindow
{
#ifdef VRAKTAL_EDITOR
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
#endif // VRAKTAL_EDITOR

	auto* instance = Input::GetInstance();
	if (instance) {
		instance->m_mousePosition = glm::vec2(xpos, ypos); // Update 
	}
}

void core::Input::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Input::GetInstance()->m_windowResize(width, height);
}

void core::Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	Input* Context = GetInstance();
	for (auto const& [key, val] : Context->ScrollBind)
	{
		val(xoffset, yoffset);
	}
}



glm::vec2* core::Input::GetMousePosition()
{
	return &LastMousePosition; // Placeholder, should return actual mouse position
}

glm::vec2* core::Input::GetMouseMovement()
{
	return &MouseMovement;
}

bool core::Input::BindActionKey(std::vector<input::Key> _keys, std::string _actionName)
{
	auto id = ActionsStorage.Find(_actionName);
	if (id != NamedStorageMap<KeyActions>::INVALID_ID)
	{
		ActionsStorage.Get(id).keys = _keys;
		return true;
	}
	return false;
}

std::vector<input::Key> core::Input::GetActionKey(std::string _name)
{
	auto id = ActionsStorage.Find(_name);
	if (id != NamedStorageMap<KeyActions>::INVALID_ID)
	{
		return ActionsStorage.Get(id).keys;
	}
	throw;
}
