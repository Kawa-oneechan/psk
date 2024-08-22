#pragma once

#include "SpecialK.h"

enum class Binds
{
	Up, Down, Left, Right,
	Accept, Back,
	PageUp, PageDown,
};
//Keep this one updated.
const int NumBinds = 8;

struct InputKey
{
	bool State;
	int ScanCode;
	std::string Name;
};

class InputsMap
{
private:
	glm::vec2 lastMousePos;

public:
	InputKey Keys[NumBinds]{ 0 };

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	InputsMap();
	void Process(int scancode, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	void Clear();

	bool KeyDown(Binds bind);
	inline void Clear(Binds bind) { Keys[(int)bind].State = false; }
};
extern InputsMap Inputs;

