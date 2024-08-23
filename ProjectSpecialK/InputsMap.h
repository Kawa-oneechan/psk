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

struct InputKey
{
	bool State;
	int ScanCode;
	std::string Name;
};

class InputsMap
{
private:
	glm::vec2 lastMousePos;

public:
	InputKey Keys[NumKeyBinds]{ 0 };

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft, MouseHoldRight, MouseHoldMiddle;
	glm::vec2 MousePosition;

	InputsMap();
	void Process(int scancode, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	void Clear();

	bool KeyDown(Binds bind);
	inline void Clear(Binds bind) { Keys[(int)bind].State = false; }
};
extern InputsMap Inputs;

