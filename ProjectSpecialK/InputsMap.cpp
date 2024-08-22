#include "InputsMap.h"

#include <GLFW/glfw3.h>

InputsMap::InputsMap()
{
	Clear();

	lastMousePos = MousePosition = glm::vec2(width, height) + 20.0f;
}

void InputsMap::Process(int scancode, int action)
{
	if (action == GLFW_PRESS)
	{
		//It's too bad we can't use switch here...
		for (auto& k : Keys)
		{
			if (scancode == k.ScanCode)
			{
				k.State = true;
				break;
			}
		}
	}
}

void InputsMap::MouseMove(float x, float y)
{
	lastMousePos = MousePosition;
	MousePosition.x = x;
	MousePosition.y = y;
}

bool InputsMap::MouseMoved()
{
	auto ret = (lastMousePos != MousePosition);
	lastMousePos = MousePosition;
	return ret;
}

void InputsMap::Clear()
{
	for (auto& k : Keys)
		k.State = false;
	MouseLeft = MouseRight = MouseMiddle = false;
	MouseHoldLeft = MouseHoldRight = MouseHoldMiddle = false;
}

bool InputsMap::KeyDown(Binds bind)
{
	return Keys[(int)bind].State;
}

InputsMap Inputs = InputsMap();

