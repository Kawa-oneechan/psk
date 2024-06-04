#pragma once

#include "SpecialK.h"


class InputsMap
{
private:
	glm::vec2 lastMousePos;

public:
	bool Up, Down, Left, Right;
	bool Enter, Escape;
	bool PgUp, PgDown;

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft;
	glm::vec2 MousePosition;

	InputsMap();
	void Process(int key, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	void Clear();
};
extern InputsMap& Inputs;

