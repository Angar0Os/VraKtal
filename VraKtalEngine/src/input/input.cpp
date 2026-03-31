#include <core/input/input.h>
#include <core/window.h>
#include <iostream>
#include <GLFW/glfw3.h>

core::Input* core::Input::m_instance = nullptr;

struct core::Input::Internal
{
	GLFWwindow* window = nullptr;
};

core::Input::Input(core::Window& window) : m_internal(new Internal)
{
	m_internal->window = static_cast<GLFWwindow*>(window.GlfwHandle());
	glfwSetKeyCallback(m_internal->window, key_callback);
	glfwSetCursorPosCallback(m_internal->window, mouse_callback);
	glfwSetFramebufferSizeCallback(m_internal->window, framebuffer_size_callback);
	glfwSetScrollCallback(m_internal->window, scroll_callback);

	m_instance = this; // Set the static instance pointer to this instance

	keyToActions.resize(348); //Init all to InvalidActions
	CurrentKeys.resize(348);
	PreviousKeys.resize(348);
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

void core::Input::Update()
{
	// Update mouse movement
	float _xoffset = m_mousePosition.x - LastMousePosition.x;
	float _yoffset = LastMousePosition.y - m_mousePosition.y; // reversed since y-coordinates go from bottom to top

	MouseMovement = glm::vec2(_xoffset, _yoffset);
	LastMousePosition = m_mousePosition;

    m_mouseBind.Execute(MouseMovement);

	Action::ActionType type;
	bool current	= false;
	bool previous	= false;
	bool shouldCall = false;

	for (size_t i = 0; i < CurrentKeys.size(); i++)
	{
		current = CurrentKeys[i];
		previous = PreviousKeys[i];

		if (current && !previous)
		{
			type = Action::Press;
			shouldCall = true;

		}
		else if (!current && previous)
		{
			type = Action::Release;
			shouldCall = true;
		}
		else if (current && previous)
		{
			type = Action::OnGoing;
			shouldCall = true;
		}

		if (shouldCall)
		{
			for (ActionID actionId : keyToActions[i])
			{
				CallAction(Actions.Get(actionId), type);
			}
		}
        PreviousKeys[i] = CurrentKeys[i];
		shouldCall = false;
	}

	for (auto axisId : Axis2DActions.GetActiveValues())
	{

		glm::vec2 value(0.0f);

        value.x += CurrentKeys[static_cast<size_t>(axisId->positiveX)] ? 1.0f : 0.0f;
        value.x -= CurrentKeys[static_cast<size_t>(axisId->negativeX)] ? 1.0f : 0.0f;
        value.y += CurrentKeys[static_cast<size_t>(axisId->positiveY)] ? 1.0f : 0.0f;
        value.y -= CurrentKeys[static_cast<size_t>(axisId->negativeY)] ? 1.0f : 0.0f;
		
        axisId->FunctionAxis2D.Execute(value);
	}

	for (auto* combo : ComboActions.GetActiveValues())
	{
		bool comboActive = true;
		for (uint32_t key : combo->keys)
		{
			if (!CurrentKeys[key])
			{
				comboActive = false;
				break;
			}
		}
		if (comboActive)
		{
			combo->FunctionCallback.Execute();
		}
    }
}

void core::Input::CallAction(Functions& _actions, int actionType)
{
	switch (actionType)
	{
	case 0: //Released
		for (const InputCallback& callback : _actions.FunctionReleased)
		{
			callback.Execute();
		}
		break;
	case 1: //Pressed
		for (const InputCallback& callback : _actions.FunctionPressed)
		{
			callback.Execute();
		}
		break;
	case 2: //OnGoing
		for (const InputCallback& callback : _actions.FunctionOnGoing)
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

void core::Input::AddAction(std::string _actionName)
{
    Actions.Add(_actionName);
}

void core::Input::RemoveAction(std::string _actionName)
{
    Actions.Remove(_actionName);
}

void core::Input::AddAxis2DAction(std::string _actionName, Keys::Key _positiveX, Keys::Key _negativeX, Keys::Key _positiveY, Keys::Key _negativeY)
{
    Axis2DActions.Add(_actionName);
    Axis2DActions.Get(Axis2DActions.Find(_actionName)).positiveX = _positiveX;
    Axis2DActions.Get(Axis2DActions.Find(_actionName)).negativeX = _negativeX;
    Axis2DActions.Get(Axis2DActions.Find(_actionName)).positiveY = _positiveY;
    Axis2DActions.Get(Axis2DActions.Find(_actionName)).negativeY = _negativeY;
}

void core::Input::RemoveAxis2DAction(std::string _actionName)
{
    Axis2DActions.Remove(_actionName);
}

void core::Input::AddComboAction(std::string _ComboName)
{
    ComboActions.Add(_ComboName);
}

void core::Input::RemoveComboAction(std::string _ComboName)
{
    ComboActions.Remove(_ComboName);
}

bool core::Input::BindActionKey(Keys::Key _key, std::string _actionName)
{
	if (!Actions.Contains(_actionName))
		return false;

	size_t keyIndex = static_cast<size_t>(_key);

	if (keyIndex >= keyToActions.size())
		return false;

	ActionID actionID = Actions.Find(_actionName);

	if (std::find(keyToActions[keyIndex].begin(), keyToActions[keyIndex].end(), actionID) != keyToActions[keyIndex].end())
		return false; // déjà bindée

	keyToActions[keyIndex].push_back(actionID);

    std::cout << "Binded Key : " << _key << " to Action : " << _actionName << std::endl;

	return true;

}

void core::Input::BindFunctionToAction(std::string _actionName, InputCallback _functionToCall, Action::ActionType _onWhat)
{
	if (!Actions.Contains(_actionName))
		return;
	
	ActionID actionID = Actions.Find(_actionName);
	switch (_onWhat)
	{
	case Action::Release:
        std::cout << "Binded Release Function to Action : " << _actionName << std::endl;
		Actions.Get(actionID).FunctionReleased.push_back(_functionToCall);
		break;
	case Action::Press:
        std::cout << "Binded Press Function to Action : " << _actionName << std::endl;
		Actions.Get(actionID).FunctionPressed.push_back(_functionToCall);
		break;
	case Action::OnGoing:
        std::cout << "Binded OnGoing Function to Action : " << _actionName << std::endl;
		Actions.Get(actionID).FunctionOnGoing.push_back(_functionToCall);
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

