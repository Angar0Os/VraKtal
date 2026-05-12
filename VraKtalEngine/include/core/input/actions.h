#pragma once
#include <vector>
#include <core/input/keys.h>

#include <core/input/InputCallback.h>
#include <core/input/inputAxisCallback.h>

struct KeyActions //vector we are going to poss through to call when needed 
{
	std::vector<InputCallback>	Press;
	std::vector<InputCallback>	Release;
	std::vector<InputCallback>	OnGoing;
	std::vector<input::Key> keys;
	
	bool enabled = true;

	KeyActions() {
		Press.resize(2);
		Release.resize(2);
		OnGoing.resize(2);
	}
};

struct Axis2DAction
{
	InputAxisCallback<glm::vec2> FunctionAxis2D;
	input::Key key[4] = {input::Key::RIGHT ,input::Key::LEFT ,input::Key::UP,input::Key::DOWN};

	bool enabled = true;
};