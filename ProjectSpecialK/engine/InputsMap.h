#pragma once
#include <string>
#include <glm/glm.hpp>
#include "../InputsMap.h"

struct InputKey
{
	bool State;
	int ScanCode;
	int GamepadButton;
	std::string Name;
};

class InputsMap
{
private:
	glm::vec2 lastMousePos;
	unsigned char trg[15]{ 0 };
	unsigned char cnt[15]{ 0 };

public:
	InputKey Keys[NumKeyBinds]{ 0 };
	bool Shift, Control, Alt;

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	bool HaveGamePad = false;
	int StickAngles[2];
	float StickDists[2];
	float RunThreshold{ 0.5f };
	float Deadzone{ 0.2f };

	InputsMap();
	void Process(int scancode, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	bool UpdateGamepad();
	void Clear(bool alsoGamepad = false);

	bool KeyDown(Binds bind);
	inline void Clear(Binds bind)
	{
		Keys[(int)bind].State = false;
		//cnt[Keys[(int)bind].GamepadButton] = 0;
	}
};

extern InputsMap Inputs;
