#pragma once
#include <vector>
#include "Tickable.h"
#include "JsonUtils.h"
#include "../Game.h"

class Console;

//Game-specific callback functions.
class Game
{
public:
	//Registers console commands and variables.
	static void RegisterConsole(Console* console);
	//Processes loading settings from JSON to variables and members.
	static void LoadSettings(jsonObject& settings);
	//Processes saving gsettings from variables and members to JSON.
	static void SaveSettings(jsonObject& settings);
	static void Initialize();
	static void Start(std::vector<TickableP>& tickables);
	static void OnMouse(double xPosIn, double yPosIn, float xoffset, float yoffset);
	static void OnResize();
	static void OnQuit();
	static void LoopStart();
	static void PreDraw(float dt);
	static void PostDraw(float dt);
#ifdef BECKETT_EXTRASAVEDIRS
	static void PrepareSaveDirs();
#endif
#ifdef DEBUG
	static void ImGui();
#endif
};