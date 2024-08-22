#pragma once

#include "SpecialK.h"

enum class Binds
{
	Up, Down, Left, Right
};

class InputsMap
{
private:
	glm::vec2 lastMousePos;

public:
	bool Up, Down, Left, Right;
	bool Enter, Escape;
	bool PgUp, PgDown;

	int Bindings[sizeof(Binds)]{ 0 };

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	InputsMap();
	void Process(int scancode, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	void Clear();
};
extern InputsMap Inputs;

