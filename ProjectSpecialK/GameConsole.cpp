#include "engine/Console.h"
#include "engine/Framebuffer.h"
#include "engine/Audio.h"
#include "engine/Game.h"
#include "engine/Shader.h"
#include "Game.h"
#include "Player.h"

extern bool debugPanelLayoutPolygons;
extern bool debugRenderPanelLayouts;
extern bool wireframe;
extern bool debuggerEnabled;
extern bool useOrthographic;
extern bool botherColliding;
extern bool showPos;

bool noWear; //placeholder

extern Framebuffer* postFxBuffer;

void CCmdReshade(const jsonArray& args)
{
	(void)(args);

	Shader::ReloadAll();
	//postFxBuffer->ReloadShader();
	{
		auto lut = postFxBuffer->GetLut();
		delete lut;
		postFxBuffer->SetLut(new TextureArray("colormap*.png"));
	}
}

void Game::RegisterConsole(Console* console)
{
#define RV console->RegisterCVar

	RV("cl_showpos", CVar::Type::Bool, &showPos, "Shows player position on screen");
#ifdef DEBUG
	RV("debugger", CVar::Type::Bool, &debuggerEnabled, "Enables ImGui debugger");
#endif
	RV("r_drawgui", CVar::Type::Bool, &debugRenderPanelLayouts, CVar::Flags::Cheat, "Render panel layouts");
	RV("r_drum", CVar::Type::Bool, &commonUniforms.CurveEnabled, "Enable camera distortion");
	RV("r_drumexp", CVar::Type::Float, &commonUniforms.CurvePower, "Camera distortion strength");
	RV("r_wireframe", CVar::Type::Bool, &wireframe, "Renders 3D models as wireframes");
	RV("r_polygons", CVar::Type::Bool, &debugPanelLayoutPolygons, "Shows outlines for clickable panel layout panels");
	RV("r_postfx", CVar::Type::Int, &commonUniforms.PostEffect, CVar::Flags::Normal, 0, 4, nullptr, "Post-processing effect");
	RV("r_colorlut", CVar::Type::Int, &commonUniforms.ColorLut, CVar::Flags::Normal, 0, 16, nullptr, "Post-processing color lookup palette");
	RV("r_toon", CVar::Type::Bool, &commonUniforms.Toon, "Enables toon shading");
	RV("r_fresnel", CVar::Type::Bool, &commonUniforms.Fresnel, "Enables fresnel effect on 3D models");
	RV("r_fresnelpow", CVar::Type::Float, &commonUniforms.FresnelPower, CVar::Flags::Normal, 0, 6, nullptr, "Strength of fresnel effect");
	RV("r_zomboid", CVar::Type::Bool, &useOrthographic, "Enables orthographic rendering");

	RV("s_ambientvolume", CVar::Type::Float, &Audio::AmbientVolume, CVar::Flags::Normal, 0, 100);
	RV("s_effectvolume", CVar::Type::Float, &Audio::SoundVolume, CVar::Flags::Normal, 0, 100);
	RV("s_musicvolume", CVar::Type::Float, &Audio::MusicVolume, CVar::Flags::Normal, 0, 100);
	RV("s_voicevolume", CVar::Type::Float, &Audio::SpeechVolume, CVar::Flags::Normal, 0, 100);

	RV("bells", CVar::Type::Int, &thePlayer.Bells, CVar::Flags::Cheat, "Number of Bells on hand");
	RV("gender", CVar::Type::Int, &thePlayer.Gender, CVar::Flags::Normal, 0, 3);
	RV("name", CVar::Type::String, &thePlayer.Name, "Player's name");
	RV("nowear", CVar::Type::Bool, &noWear, CVar::Flags::Cheat, "Disables wear and tear on tools");
	RV("colorskin", CVar::Type::Color, &thePlayer.SkinTone, "Player's skintone");
	RV("colorskinedge", CVar::Type::Color, &thePlayer.SkinEdge, "Player's skintone");
	RV("colorhair", CVar::Type::Color, &thePlayer.HairColor, "Player's hair color");
	RV("colorhairlite", CVar::Type::Color, &thePlayer.HairHiliteColor, "Player's hair color");
	RV("colorcheeks", CVar::Type::Color, &thePlayer.CheekColor, "Player's cheek color");
	RV("coloreyes", CVar::Type::Color, &thePlayer.EyeColor, "Player's eye color");

	//TODO: add a change callback to handle the snow/grass change.
	RV("grass", CVar::Type::Float, &commonUniforms.GrassColor, "Grass color in progression through the year");

	//replace this with noclip below
	RV("collidenpc", CVar::Type::Bool, &botherColliding, false);

	//RV("ai_disable", CVar::Type::Bool, &);
	//RV("cl_showpos", CVar::Type::Bool, &);
	//RV("noclip", CVar::Type::Bool, &, true);
	//RV("r_acredistance", CVar::Type::Int, &, false, 1, 6);
	//RV("r_farz", CVar::Type::float, &, yes);

#undef RV

	console->RegisterCCmd("reshade", CCmdReshade, false, "Forces reload of all shaders");
}
