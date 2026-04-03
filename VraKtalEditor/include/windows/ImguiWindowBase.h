#pragma once
class ImguiWindowBase
{
public:
	ImguiWindowBase() = default;
	~ImguiWindowBase() = default;

	virtual void Draw() = 0;
};