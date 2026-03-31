#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

#include <glm/glm.hpp>
#include <utils/NamedStorageMapped.h>
#include "../input/InputCallback.h"
#include "../input/inputAxisCallback.h"
#include <iostream>

/*Forward Declarations*/
struct	GLFWwindow;

namespace Keys
{
	enum Key
	{
		SPACE = 32,
		APOSTROPHE = 39,/* ' */
		COMMA = 44,/* , */
		MINUS = 45,/* - */
		PERIOD = 46,/* . */
		SLASH = 47,/* / */
		num_0 = 48,
		num_1 = 49,
		num_2 = 50,
		num_3 = 51,
		num_4 = 52,
		num_5 = 53,
		num_6 = 54,
		num_7 = 55,
		num_8 = 56,
		num_9 = 57,
		SEMICOLON = 59,/* ; */
		EQUAL = 61,/* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LEFT_BRACKET = 91,/* [ */
		BACKSLASH = 92,/* \ */
		RIGHT_BRACKET = 93,/* ] */
		GRAVE_ACCENT = 96,/* ` */
		WORLD_1 = 161, /* non-US #1 */
		WORLD_2 = 162, /* non-US #2 */
		ESCAPE = 256,
		ENTER = 257,
		TAB = 258,
		BACKSPACE = 259,
		INSERT = 260,
		RIGHT = 262,
		LEFT = 263,
		DOWN = 264,
		UP = 265,
		PAGE_UP = 266,
		PAGE_DOWN = 267,
		HOME = 268,
		END = 269,
		CAPS_LOCK = 280,
		SCROLL_LOCK = 281,
		NUM_LOCK = 282,
		PRINT_SCREEN = 283,
		PAUSE = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		KP_0 = 320,
		KP_1 = 321,
		KP_2 = 322,
		KP_3 = 323,
		KP_4 = 324,
		KP_5 = 325,
		KP_6 = 326,
		KP_7 = 327,
		KP_8 = 328,
		KP_9 = 329,
		KP_DECIMAL = 330,
		KP_DIVIDE = 331,
		KP_MULTIPLY = 332,
		KP_SUBTRACT = 333,
		KP_ADD = 334,
		KP_ENTER = 335,
		KP_EQUAL = 336,
		LEFT_SHIFT = 340,
		LEFT_CONTROL = 341,
		LEFT_ALT = 342,
		LEFT_SUPER = 343,
		RIGHT_SHIFT = 344,
		RIGHT_CONTROL = 345,
		RIGHT_ALT = 346,
		RIGHT_SUPER = 347
	};
};

namespace Action
{
	enum ActionType
	{
		Release = 0,
		Press	= 1,
		OnGoing = 2
	};
}

namespace core {
	class	Window;

	class Input
	{
	public:
		explicit Input(core::Window& window);
		~Input();
	
		void Update();

		typedef void (Input::* mouseCallback)(double x, double y);
		
		template<typename T, void(T::* Method)()>
		void BindActionCallback(std::string actionName, T* instance, Action::ActionType onWhat)
		{
			InputCallback action;
			action.context = instance;
			action.callback = &MethodCaller<T, Method>;
			BindFunctionToAction(actionName, action, onWhat);
		}

        template<typename T, void(T::* Method)(glm::vec2)>
		void BindAxis2DCallack(std::string actionName, T* instance)
		{
			InputAxisCallback<glm::vec2> action;
            action.context = instance;
            action.callback = &MethodCaller<T, Method>;
			if (!Axis2DActions.Contains(actionName))
                return;
            Axis2DActions.Get(Axis2DActions.Find(actionName)).FunctionAxis2D = action;
		}

		template<typename T, void(T::* Method)(glm::vec2)>
		void BindMouseCallback(T* instance)
		{
            m_mouseBind.context = instance;
            m_mouseBind.callback = &MethodCaller<T, Method>;
            std::cout << "Binded Mouse Callback to Function" << std::endl;
        }

        template<typename T, void(T::* Method)()>
		void BindComboAction(std::string name , std::vector<Keys::Key> keys, T* instance)
		{
			if (!ComboActions.Contains(name))
				return;

			InputCallback action;
			action.context = instance;
			action.callback = &MethodCaller<T, Method>;

            ComboFunction comboFunction;
            comboFunction.FunctionCallback = action;
            comboFunction.keys = keys;

            ComboActions.Get(ComboActions.Find(name)) = comboFunction;
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
	
	public:
		void AddAction(std::string _actionName);		// Add an action ex: "MoveForward"
		void RemoveAction(std::string _actionName);
		bool BindActionKey(Keys::Key _key, std::string _actionName); //Bind key to action

        void AddAxis2DAction(std::string _actionName, Keys::Key _positiveX, Keys::Key _negativeX, Keys::Key _positiveY, Keys::Key _negativeY); // Add an axis 2D action ex: "Move" with WASD
        void RemoveAxis2DAction(std::string _actionName);
		
        void AddComboAction(std::string _ComboName);
		void RemoveComboAction(std::string _ComboName);

		void AddScrollAction(void* object, void(*func)(double, double)); //nique
		void RemoveScrollAction(void* object, void(*func)(double, double));
	
		glm::vec2* GetMousePosition(); //Get mouse position
		glm::vec2 LastMousePosition; //Mouse position
		glm::vec2* GetMouseMovement(); //Get mouse movement since last frame
		glm::vec2 MouseMovement; //Mouse movement since last frame
	
	private:
		void BindFunctionToAction(std::string _actionName, InputCallback _functionToCall, Action::ActionType _onWhat); // Bind Function to an action
		
		static Input* m_instance;
		struct Internal;
		std::unique_ptr<Internal> m_internal;
	
		using ActionID = size_t;

		struct Functions //vector we are going to poss through to call when needed 
		{
			std::vector<InputCallback>	FunctionPressed;
			std::vector<InputCallback>	FunctionReleased;
			std::vector<InputCallback>	FunctionOnGoing;
		};

		struct Axis2DFunction
		{
            InputAxisCallback<glm::vec2> FunctionAxis2D;
            Keys::Key positiveX;
            Keys::Key negativeX;
            Keys::Key positiveY;
            Keys::Key negativeY;
		};

		struct ComboFunction
		{
			InputCallback FunctionCallback;
            std::vector<Keys::Key> keys;
		};

		NamedStorageMap<Axis2DFunction> Axis2DActions;

		NamedStorageMap<Functions> Actions;

        NamedStorageMap<ComboFunction> ComboActions;

		std::vector<std::vector<ActionID>> keyToActions;		  // Key -> ActionIDs

		std::vector<bool> CurrentKeys;
		std::vector<bool> PreviousKeys;
	
		std::map<void*, void(*)(double, double)> ScrollBind;
	
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods); // glfwWindow
	
        InputAxisCallback<glm::vec2> m_mouseBind; // Mouse movement callback
		static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
		void(*m_windowResize)(int, int);
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		void CallAction(Functions& _actions , int actionType);
	
		static Input* GetInstance();
		static void DestroyInstance();
	
		glm::vec2 m_mousePosition; // Current mouse position


	};
}