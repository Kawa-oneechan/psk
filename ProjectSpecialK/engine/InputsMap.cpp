#include <GLFW/glfw3.h>
#include "InputsMap.h"
#include "../Game.h"

extern int width, height;

InputsMap::InputsMap()
{
	Clear();

	lastMousePos = MousePosition = glm::vec2(width, height) + 20.0f;
	MouseHoldLeft = MouseHoldMiddle = MouseHoldRight = false;
	Shift = Control = Alt = false;
	StickAngles[0] = StickAngles[1] = 0;
	StickDists[0] = StickDists[1] = 0.0f;
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
	else if (action == GLFW_RELEASE)
	{
		for (auto& k : Keys)
		{
			if (scancode == k.ScanCode)
			{
				k.State = false;
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
		const float dead = 0.2f;

		for (int i = 0; i < 2; i++)
		{
			auto x = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X + (i * 2)];
			auto y = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y + (i * 2)];
			if (glm::abs(x) + glm::abs(y) > dead)
			{
				StickAngles[i] = (360 + (int)glm::degrees(std::atan2f(y, x)) + 90) % 360;
				auto dotdotdot = glm::vec2(x, y);
				StickDists[i] = glm::dot(dotdotdot, dotdotdot);
				if (i == 0)
					Shift = StickDists[0] > RunThreshold;
			}
			else
			{
				StickAngles[i] = 0;
				StickDists[i] = 0.0f;
			}
		}

#ifdef BECKETT_ANALOGLEFT
		Keys[(int)BECKETT_ANALOGLEFT + 0].State = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] < -dead;
		Keys[(int)BECKETT_ANALOGLEFT + 1].State = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] < -dead;
		Keys[(int)BECKETT_ANALOGLEFT + 2].State = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] > dead;
		Keys[(int)BECKETT_ANALOGLEFT + 3].State = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X] > dead;
		//TODO: Hold Shift if the axes are pushed further to run. Will need my gamepad to test.
#endif

#ifdef BECKETT_ANALOGRIGHT
		Keys[(int)BECKETT_ANALOGRIGHT + 0].State = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] < -dead;
		Keys[(int)BECKETT_ANALOGRIGHT + 1].State = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] < -dead;
		Keys[(int)BECKETT_ANALOGRIGHT + 2].State = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] > dead;
		Keys[(int)BECKETT_ANALOGRIGHT + 3].State = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] > dead;
#endif

		/*
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
		*/

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
	for (int i = 0; i < NumKeyBinds; i++)
	{
		if (i >= (int)Binds::WalkN && i <= (int)Binds::WalkE)
			continue;
		Keys[i].State = false;
	}
	if (alsoGamepad)
		for (int i = 0; i < 15; i++)
			cnt[i] = trg[i] = 0;
	MouseLeft = MouseRight = MouseMiddle = false;
	//MouseHoldLeft = MouseHoldRight = MouseHoldMiddle = false;
}

bool InputsMap::KeyDown(Binds bind)
{
	return Keys[(int)bind].State;
}

InputsMap Inputs;
