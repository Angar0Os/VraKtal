#pragma once

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <map>
#include <functional>

#include <glm/glm.hpp>

#include <utils/NamedStorageMapped.h>
#include <core/input/keys.h>
#include <core/input/actions.h>

/*Forward Declarations*/
struct	GLFWwindow;

namespace core {
	class	Window;
	namespace gpu {
		class Device;
	}


	class Input
	{
	private:

		NamedStorageMap<Axis2DAction> Axis2DActionsStorage;
		NamedStorageMap<KeyActions>	ActionsStorage;

	public:
		explicit Input(core::Window& window, core::gpu::Device* _device);
		~Input();
		void Update();

		typedef void (Input::* mouseCallback)(double x, double y);

		template<typename T, void(T::* Method)()>
		void BindActionCallback(std::string actionName, T* instance, input::KeyState onWhat)
		{
			InputCallback action;
			action.context = instance;
			action.callback = &MethodCaller<T, Method>;
			std::cout << "Binding " << actionName << " to " << input::KeyStateToString(onWhat) << std::endl;
			BindFunctionToAction(actionName, action, onWhat);
		}

		template<typename T, void(T::* Method)(glm::vec2)>
		void BindAxis2DCallack(std::string actionName, T* instance)
		{
			InputAxisCallback<glm::vec2> action;
			action.context = instance;
			action.callback = &MethodCaller<T, Method>;
			if (!Axis2DActionsStorage.Contains(actionName))
				return;
			Axis2DActionsStorage.Get(Axis2DActionsStorage.Find(actionName)).FunctionAxis2D = action;
			std::cout << "Binded Axis Function to Action : " << actionName << std::endl;
		}

		template<typename T, void(T::* Method)(glm::vec2)>
		void BindMouseCallback(T* instance)
		{
			m_mouseBind.context = instance;
			m_mouseBind.callback = &MethodCaller<T, Method>;
			std::cout << "Binded Mouse Callback to Function" << std::endl;
		}

	private:
		template<typename T, void(T::* Method)()>
		static void MethodCaller(void* context)
		{
			T* obj = static_cast<T*>(context);
			(obj->*Method)();
		}

		template<typename T, void(T::* Method)(glm::vec2)>
		static void MethodCaller(void* context, const glm::vec2& value)
		{
			T* obj = static_cast<T*>(context);
			(obj->*Method)(value);
		}

	public: //User Side
		void AddAction(std::string _actionName);						// Add an action ex: "MoveForward"
		void RemoveAction(std::string _actionName);
		bool BindActionKey(std::vector<input::Key> _keys, std::string _actionName, bool enable = true);
		bool ToggleAction(std::string _actionName);


		void AddAxis2DAction(std::string _actionName, input::Key _positiveX, input::Key _negativeX, input::Key _positiveY, input::Key _negativeY, bool enable = true); // Add an axis 2D action ex: "Move" with WASD
		void RemoveAxis2DAction(std::string _actionName);
		bool ToggleAxis2DAction(std::string _actionName);

		NamedStorageMap<KeyActions>& GetActionsStorage() { return ActionsStorage; };
		NamedStorageMap<Axis2DAction>& GetAxisStorage() { return Axis2DActionsStorage; }

		void AddScrollAction(void* object, void(*func)(double, double));
		void RemoveScrollAction(void* object, void(*func)(double, double));

		glm::vec2* GetMousePosition();	//Get mouse position
		glm::vec2 LastMousePosition;	//Mouse position
		glm::vec2* GetMouseMovement();	//Get mouse movement since last frame
		glm::vec2 MouseMovement;		//Mouse movement since last frame

		std::vector<input::Key> GetActionKey(std::string _name);
		std::vector<int> GetCurrentKeyPressed();
	private:
		void BindFunctionToAction(std::string _actionName, InputCallback _functionToCall, input::KeyState _onWhat); // Bind Function to an action

		static Input* m_instance;
		struct Internal;
		std::unique_ptr<Internal> m_internal;

		using ActionID = size_t;

		std::vector<bool> CurrentKeys;
		std::vector<bool> PreviousKeys;

		std::map<void*, void(*)(double, double)> ScrollBind;

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // glfwWindow

		InputAxisCallback<glm::vec2> m_mouseBind; // Mouse movement callback
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

		std::function<void(int, int)> m_windowResize; // ← std::function, initialisé dans le constructeur
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		void CallAction(const KeyActions* _actions, input::KeyState actionType);

		static Input* GetInstance();
		static void DestroyInstance();

		glm::vec2 m_mousePosition; // Current mouse position

		input::KeyState GetBindingState(const std::vector<input::Key>& keys) const;

		core::Window* m_windowRef = nullptr;
		core::gpu::Device* m_device;

	};
}