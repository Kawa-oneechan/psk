#pragma once

#include "SpecialK.h"

//For keycodes in Default Bindings.
#include <GLFW/glfw3.h>

enum class Binds
{
	Up, Down, Left, Right,
	Accept, Back,
	PageUp, PageDown,
	Interact, PickUp,
	Inventory, Map, React,
	HotBar1, HotBar2, HotBar3, HotBar4, HotBar5,
	HotBar6, HotBar7, HotBar8, HotBar9, HotBar10,

};
constexpr int DefaultInputBindings[] = {
	GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
	GLFW_KEY_ENTER, GLFW_KEY_ESCAPE,
	GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN,
	GLFW_KEY_Z, GLFW_KEY_Q,
	GLFW_KEY_TAB, GLFW_KEY_M, GLFW_KEY_R,
	GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3, GLFW_KEY_F4, GLFW_KEY_F5,
	GLFW_KEY_F6, GLFW_KEY_F7, GLFW_KEY_F8, GLFW_KEY_F9, GLFW_KEY_F10
};
constexpr int NumKeyBinds = sizeof(DefaultInputBindings) / sizeof(int);

constexpr int DefaultInputGamepadBindings[] = {
	GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
	GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B,
	GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
	GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_Y,
	GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
	-1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1,
};

//Maps GLFW gamepad buttons to Private Use Area characters. Note that these are raw codepoints, NOT UTF-8.
constexpr int GamepadPUAMap[] =
{
	//ABXY
	0xE0E0, 0xE0E1, 0xE0E2, 0xE0E3,
	//Shoulders
	0xE0E4, 0xE0E5,
	//Back, Start, Guide
	0xE0F2, 0xE0F1, 0xE0F4,
	//Thumbs
	0xE101, 0xE102,
	//DPad
	0xE0EB, 0xE0EE, 0xE0EC, 0xE0ED,

	/* Outlined:
	//ABXY
	0xE0A0, 0xE0A1, 0xE0A2, 0xE0A3,
	//Shoulders
	0xE0A4, 0xE0A5,
	//Back, Start, Guide
	0xE0B4, 0xE0B3, 0xE0B9,
	//Thumbs
	0xE0C1, 0xE0C2,
	//DPad
	0xE0AF, 0xE0B2, 0xE0B0, 0xE0B1,
	*/
};
constexpr int NumPUAMaps = sizeof(GamepadPUAMap) / sizeof(int);

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
	unsigned char trg[15];
	unsigned char cnt[15];
	
public:
	InputKey Keys[NumKeyBinds]{ 0 };

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	bool HaveGamePad;

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

