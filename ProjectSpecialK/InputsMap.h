#pragma once
#include <string>

//For keycodes in Default Bindings.
#include "engine/GlfwKeys.h"

enum class Binds
{
	//Cursor controls
	Up, Down, Left, Right,
	Accept, Back,
	PageUp, PageDown,
	//Player controls
	WalkN, WalkW, WalkS, WalkE,
	Interact, PickUp,
	//Camera controls
	CameraCW, CameraCCW, CameraUp, CameraDown,
	//Functions
	Inventory, Map, React,
	//Hotbar
	HotBar1, HotBar2, HotBar3, HotBar4, HotBar5,
	HotBar6, HotBar7, HotBar8, HotBar9, HotBar10,
	//System stuff
	Console, Screenshot
};
constexpr int DefaultInputBindings[] = {
	//Cursor controls
	GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
	GLFW_KEY_ENTER, GLFW_KEY_ESCAPE,
	GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN,
	//Player controls
	GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
	GLFW_KEY_E, GLFW_KEY_Q,
	//Camera controls
	GLFW_KEY_J, GLFW_KEY_L, GLFW_KEY_I, GLFW_KEY_K,
	//Functions
	GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3,
	//Hotbar
	GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
	GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9, GLFW_KEY_0,
	//System stuff
	GLFW_KEY_GRAVE_ACCENT, GLFW_KEY_F12
};
constexpr int NumKeyBinds = sizeof(DefaultInputBindings) / sizeof(int);

constexpr int DefaultInputGamepadBindings[] = {
	//Cursor controls
	GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
	GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_B,
	GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
	//Player controls
	GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
	GLFW_GAMEPAD_BUTTON_A, GLFW_GAMEPAD_BUTTON_Y,
	//Camera controls
	-1, -1, -1, -1,
	//Functions
	GLFW_GAMEPAD_BUTTON_X, GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
	//Hotbar
	-1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1,
	//System stuff
	-1, -1
};

//Maps GLFW gamepad buttons to Private Use Area characters.
const std::string GamepadPUAMap[] =
{
	//ABXY
	u8"\uE004", u8"\uE006", u8"\uE01E", u8"\uE020",
	//Shoulders
	u8"\uE043", u8"\uE049",
	//Back, Start, Guide
	u8"\uE008", u8"\uE018", u8"\uE041",
	//Thumbs
	u8"\uE04F", u8"\uE057",
	//DPad
	u8"\uE035", u8"\uE02B", u8"\uE024", u8"\uE028",

	/* Outlined:
	//ABXY
	u8"\uE005", u8"\uE007", u8"\uE01F", u8"\uE021",
	//Shoulders
	u8"\uE044", u8"\uE04A",
	//Back, Start, Guide
	u8"\uE009", u8"\uE01C", u8"\uE042",
	//Thumbs
	u8"\uE050", u8"\uE058",
	//DPad
	u8"\uE036", u8"\uE029", u8"\uE025", u8"\uE029",
	*/
};
