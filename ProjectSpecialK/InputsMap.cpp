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
		for (auto& k : Keys)
		{
			if (scancode == k.ScanCode)
			{
				k.State = true;
				//break;
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

bool InputsMap::UpdateGamepad()
{
	if (!HaveGamePad)
		return false;

	GLFWgamepadstate state;
	if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
	{
		//Hardwire the left stick to work for the walking actions
		if      (state.axes[0] < -0.2) Keys[(int)Binds::WalkW].State = true;
		else if (state.axes[0] >  0.2) Keys[(int)Binds::WalkE].State = true;
		if      (state.axes[1] < -0.2) Keys[(int)Binds::WalkN].State = true;
		else if (state.axes[1] >  0.2) Keys[(int)Binds::WalkS].State = true;

		//Hardwire the left stick to work for the camera controls
		if      (state.axes[0] < -0.2) Keys[(int)Binds::CameraCW].State = true;
		else if (state.axes[0] >  0.2) Keys[(int)Binds::CameraCCW].State = true;
		if      (state.axes[1] < -0.2) Keys[(int)Binds::CameraUp].State = true;
		else if (state.axes[1] >  0.2) Keys[(int)Binds::CameraDown].State = true;

		for (int i = 0; i < 15; i++)
		{
			trg[i] = state.buttons[i] & (state.buttons[i] ^ cnt[i]);
			cnt[i] = state.buttons[i];
		}

		for (auto& k : Keys)
		{
			if (k.GamepadButton != -1 && trg[k.GamepadButton])
			{
				k.State = true;
				//break;
			}
		}
		return true;
	}
	return false;
}

void InputsMap::Clear(bool alsoGamepad)
{
	for (auto& k : Keys)
		k.State = false;
	if (alsoGamepad)
		for (int i = 0; i < 15; i++)
			cnt[i] = trg[i] = 0;
	MouseLeft = MouseRight = MouseMiddle = false;
	MouseHoldLeft = MouseHoldRight = MouseHoldMiddle = false;
}

bool InputsMap::KeyDown(Binds bind)
{
	return Keys[(int)bind].State;
}

InputsMap Inputs;

