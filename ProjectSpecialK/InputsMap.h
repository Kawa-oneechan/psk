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
	Console
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
	GLFW_KEY_GRAVE_ACCENT
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
	-1
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
