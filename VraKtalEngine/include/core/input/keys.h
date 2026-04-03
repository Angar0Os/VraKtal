#pragma once
namespace input
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
		RIGHT_SUPER = 347,
		GLFW_MOUSE_BUTTON_1 = 0 + 347,
		GLFW_MOUSE_BUTTON_2 = 1 + 347,
		GLFW_MOUSE_BUTTON_3 = 2 + 347,
		GLFW_MOUSE_BUTTON_4 = 3 + 347,
		GLFW_MOUSE_BUTTON_5 = 4 + 347,
		GLFW_MOUSE_BUTTON_6 = 5 + 347,
		GLFW_MOUSE_BUTTON_7 = 6 + 347,
		GLFW_MOUSE_BUTTON_8 = 7 + 347,
		GLFW_MOUSE_BUTTON_LAST = GLFW_MOUSE_BUTTON_8,
		GLFW_MOUSE_BUTTON_LEFT = GLFW_MOUSE_BUTTON_1,
		GLFW_MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_2,
		GLFW_MOUSE_BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_3
	};
	enum KeyState
	{
		Up = 0,
		Release = 1,
		Press = 2,
		OnGoing = 3
	};
	inline static const char* KeyToString(input::Key key)
	{
		switch (key)
		{
		case input::SPACE: return "Space";
		case input::APOSTROPHE: return "'";
		case input::COMMA: return ",";
		case input::MINUS: return "-";
		case input::PERIOD: return ".";
		case input::SLASH: return "/";

		case input::num_0: return "0";
		case input::num_1: return "1";
		case input::num_2: return "2";
		case input::num_3: return "3";
		case input::num_4: return "4";
		case input::num_5: return "5";
		case input::num_6: return "6";
		case input::num_7: return "7";
		case input::num_8: return "8";
		case input::num_9: return "9";

		case input::SEMICOLON: return ";";
		case input::EQUAL: return "=";

		case input::A: return "A";
		case input::B: return "B";
		case input::C: return "C";
		case input::D: return "D";
		case input::E: return "E";
		case input::F: return "F";
		case input::G: return "G";
		case input::H: return "H";
		case input::I: return "I";
		case input::J: return "J";
		case input::K: return "K";
		case input::L: return "L";
		case input::M: return "M";
		case input::N: return "N";
		case input::O: return "O";
		case input::P: return "P";
		case input::Q: return "Q";
		case input::R: return "R";
		case input::S: return "S";
		case input::T: return "T";
		case input::U: return "U";
		case input::V: return "V";
		case input::W: return "W";
		case input::X: return "X";
		case input::Y: return "Y";
		case input::Z: return "Z";

		case input::LEFT_BRACKET: return "[";
		case input::BACKSLASH: return "\\";
		case input::RIGHT_BRACKET: return "]";
		case input::GRAVE_ACCENT: return "`";

		case input::WORLD_1: return "World_1";
		case input::WORLD_2: return "World_2";

		case input::ESCAPE: return "Escape";
		case input::ENTER: return "Enter";
		case input::TAB: return "Tab";
		case input::BACKSPACE: return "Backspace";
		case input::INSERT: return "Insert";

		case input::RIGHT: return "Right";
		case input::LEFT: return "Left";
		case input::DOWN: return "Down";
		case input::UP: return "Up";

		case input::PAGE_UP: return "Page Up";
		case input::PAGE_DOWN: return "Page Down";
		case input::HOME: return "Home";
		case input::END: return "End";

		case input::CAPS_LOCK: return "Caps Lock";
		case input::SCROLL_LOCK: return "Scroll Lock";
		case input::NUM_LOCK: return "Num Lock";
		case input::PRINT_SCREEN: return "Print Screen";
		case input::PAUSE: return "Pause";

		case input::F1: return "F1";
		case input::F2: return "F2";
		case input::F3: return "F3";
		case input::F4: return "F4";
		case input::F5: return "F5";
		case input::F6: return "F6";
		case input::F7: return "F7";
		case input::F8: return "F8";
		case input::F9: return "F9";
		case input::F10: return "F10";
		case input::F11: return "F11";
		case input::F12: return "F12";
		case input::F13: return "F13";
		case input::F14: return "F14";
		case input::F15: return "F15";
		case input::F16: return "F16";
		case input::F17: return "F17";
		case input::F18: return "F18";
		case input::F19: return "F19";
		case input::F20: return "F20";
		case input::F21: return "F21";
		case input::F22: return "F22";
		case input::F23: return "F23";
		case input::F24: return "F24";
		case input::F25: return "F25";

		case input::KP_0: return "KP_0";
		case input::KP_1: return "KP_1";
		case input::KP_2: return "KP_2";
		case input::KP_3: return "KP_3";
		case input::KP_4: return "KP_4";
		case input::KP_5: return "KP_5";
		case input::KP_6: return "KP_6";
		case input::KP_7: return "KP_7";
		case input::KP_8: return "KP_8";
		case input::KP_9: return "KP_9";
		case input::KP_DECIMAL: return "KP_Decimal";
		case input::KP_DIVIDE: return "KP_Divide";
		case input::KP_MULTIPLY: return "KP_Multiply";
		case input::KP_SUBTRACT: return "KP_Subtract";
		case input::KP_ADD: return "KP_Add";
		case input::KP_ENTER: return "KP_Enter";
		case input::KP_EQUAL: return "KP_Equal";

		case input::LEFT_SHIFT: return "Left Shift";
		case input::LEFT_CONTROL: return "Left Control";
		case input::LEFT_ALT: return "Left Alt";
		case input::LEFT_SUPER: return "Left Super";
		case input::RIGHT_SHIFT: return "Right Shift";
		case input::RIGHT_CONTROL: return "Right Control";
		case input::RIGHT_ALT: return "Right Alt";
		case input::RIGHT_SUPER: return "Right Super";

		default: return "Unknown Key";
		}
	}
	inline static const char* KeyStateToString(input::KeyState state)
	{
		switch (state)
		{
		case input::Up: return "Up";
		case input::Release: return "Release";
		case input::Press: return "Press";
		case input::OnGoing: return "OnGoing";
		default: return "Unknown KeyState";
		}
	}
};
