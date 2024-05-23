#include "InputsMap.h"


InputsMap::InputsMap()
{
	Up = Down = Left = Right = false;
	Enter = Escape = false;

	lastMousePos = MousePosition = glm::vec2(width, height) + 20.0f;
	MouseLeft = MouseRight = MouseMiddle = false;
	MouseHoldLeft = false;
}

void InputsMap::Process(int key, int action)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_UP: Up = true; break;
		case GLFW_KEY_DOWN: Down = true; break;
		case GLFW_KEY_LEFT: Left = true; break;
		case GLFW_KEY_RIGHT: Right = true; break;
		case GLFW_KEY_ENTER: Enter = true; break;
		case GLFW_KEY_ESCAPE: Escape = true; break;
		default: break;
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
	Up = Down = Left = Right = Enter = Escape = false;
	MouseLeft = MouseRight = MouseMiddle = false;
}
InputsMap& Inputs = InputsMap();

