#include "engine/Console.h"
#include "engine/Framebuffer.h"
#include "engine/Audio.h"
#include "Game.h"
#include "Player.h"
#include <sol.hpp>

extern sol::state Sol;

extern bool debugPanelLayoutPolygons;
extern bool debugRenderPanelLayouts;
extern bool wireframe;
extern bool debuggerEnabled;
extern bool useOrthographic;
extern bool botherColliding;
extern bool showPos;

bool noWear; //placeholder

extern Framebuffer* postFxBuffer;

void CCmdReshade(jsonArray& args)
{
	args;

	Shader::ReloadAll();
	//postFxBuffer->ReloadShader();
	{
		auto lut = postFxBuffer->GetLut();
		delete lut;
		postFxBuffer->SetLut(new Texture("colormap.png"));
	}
}

void ConsoleRegister(Console* console)
{
#define RV console->RegisterCVar

	RV("cl_showpos", CVar::Type::Bool, &showPos);
#ifdef DEBUG
	RV("debugger", CVar::Type::Bool, &debuggerEnabled);
#endif
	RV("r_drawgui", CVar::Type::Bool, &debugRenderPanelLayouts, true);
	RV("r_drum", CVar::Type::Bool, &commonUniforms.CurveEnabled);
	RV("r_drumexp", CVar::Type::Float, &commonUniforms.CurvePower);
	RV("r_wireframe", CVar::Type::Bool, &wireframe);
	RV("r_polygons", CVar::Type::Bool, &debugPanelLayoutPolygons);
	RV("r_postfx", CVar::Type::Int, &commonUniforms.PostEffect, false, 0, 4);
	RV("r_toon", CVar::Type::Bool, &commonUniforms.Toon);
	RV("r_zomboid", CVar::Type::Bool, &useOrthographic);
	RV("s_ambientvolume", CVar::Type::Float, &Audio::AmbientVolume, false, 0, 100);
	RV("s_effectvolume", CVar::Type::Float, &Audio::SoundVolume, false, 0, 100);
	RV("s_musicvolume", CVar::Type::Float, &Audio::MusicVolume, false, 0, 100);
	RV("s_voicevolume", CVar::Type::Float, &Audio::SpeechVolume, false, 0, 100);

	RV("bells", CVar::Type::Int, &thePlayer.Bells, true);
	RV("gender", CVar::Type::Int, &thePlayer.Gender, false, 0, 3);
	RV("name", CVar::Type::String, &thePlayer.Name);
	RV("nowear", CVar::Type::Bool, &noWear, true);

	RV("grass", CVar::Type::Float, &commonUniforms.GrassColor, false);

	//replace this with noclip below
	RV("collidenpc", CVar::Type::Bool, &botherColliding, false);

	//RV("ai_disable", CVar::Type::Bool, &);
	//RV("cl_showpos", CVar::Type::Bool, &);
	//RV("noclip", CVar::Type::Bool, &, true);
	//RV("r_acredistance", CVar::Type::Int, &, false, 1, 6);
	//RV("r_farz", CVar::Type::float, &, yes);

#undef RV

	console->RegisterCCmd("reshade", CCmdReshade);
}
