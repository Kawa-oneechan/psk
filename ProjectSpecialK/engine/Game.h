#pragma once
#include <vector>
#include "Tickable.h"
#include "Console.h"
#include "JsonUtils.h"
#include "../Game.h"

class Game
{
public:
	static void RegisterConsole(Console* console);
	static void LoadSettings(jsonObject& settings);
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