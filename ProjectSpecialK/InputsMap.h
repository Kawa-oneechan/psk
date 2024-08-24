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

//Maps GLFW gamepad buttons to Private Use Area characters.
const std::string GamepadPUAMap[] =
{
	//ABXY
	u8"\uE0E0", u8"\uE0E1", u8"\uE0E2", u8"\uE0E3",
	//Shoulders
	u8"\uE0E4", u8"\uE0E5",
	//Back, Start, Guide
	u8"\uE0F2", u8"\uE0F1", u8"\uE0F4",
	//Thumbs
	u8"\uE101", u8"\uE102",
	//DPad
	u8"\uE0EB", u8"\uE0EE", u8"\uE0EC", u8"\uE0ED",

	/* Outlined:
	//ABXY
	u8"\uE0A0", u8"\uE0A1", u8"\uE0A2", u8"\uE0A3",
	//Shoulders
	u8"\uE0A4", u8"\uE0A5",
	//Back, Start, Guide
	u8"\uE0B4", u8"\uE0B3", u8"\uE0B9",
	//Thumbs
	u8"\uE0C1", u8"\uE0C2",
	//DPad
	u8"\uE0AF", u8"\uE0B2", u8"\uE0B0", u8"\uE0B1",
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
	unsigned char trg[15]{ 0 };
	unsigned char cnt[15]{ 0 };
	
public:
	InputKey Keys[NumKeyBinds]{ 0 };

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	bool HaveGamePad = false;

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

