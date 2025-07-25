#include "Game.h"
#include "Audio.h"
#include "DialogueBox.h"
#include "Camera.h"
#include "Messager.h"
#include "MusicManager.h"
#include "Town.h"
#include "Framebuffer.h"
#include "InGame.h"
#include "TitleScreen.h"
#include "Utilities.h"

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;

std::shared_ptr<DialogueBox> dlgBox = nullptr;
Audio* bgm = nullptr;
sol::state Sol;

std::shared_ptr<Camera> MainCamera;
std::shared_ptr<Messager> messager;
std::shared_ptr<MusicManager> musicManager;
std::shared_ptr<Town> town;

Framebuffer* postFxBuffer;

std::shared_ptr<Texture> cloudImage, starsImage, skyImage;

extern bool botherColliding;

namespace SolBinds
{
	extern void Setup();
}

namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	std::shared_ptr<Texture> controls{ nullptr };

	JSONObject json = JSONObject();
	JSONObject settings = JSONObject();

	std::string initFile = "init.json";

	void Load()
	{
		auto doc = VFS::ReadJSON("ui/ui.json");
		if (!doc)
			FatalError("Could not read ui/ui.json. Something is very wrong.");
		json = doc->AsObject();
		auto colors = json["colors"]->AsObject();
		for (auto& ink : colors["theme"]->AsObject())
		{
			themeColors[ink.first] = GetJSONColor(ink.second);
		}
		for (auto& ink : colors["text"]->AsArray())
		{
			textColors.push_back(GetJSONColor(ink));
		}

		try
		{
			settings = VFS::ReadSaveJSON("options.json")->AsObject();
		}
		catch (std::runtime_error&)
		{
			settings = JSON::Parse("{}")->AsObject();
		}

#define DS(K, V) if (!settings[K]) settings[K] = new JSONValue(V)
		DS("screenWidth", ScreenWidth);
		DS("screenHeight", ScreenHeight);
		DS("language", "USen");
		DS("continue", (int)LoadSpawnChoice::FrontDoor);
		DS("speech", (int)DialogueBox::Sound::Bebebese);
		DS("pingRate", 3);
		DS("balloonChance", 15);
		DS("cursorScale", 100);
		DS("botherColliding", true);
		DS("24hour", true);
		DS("contentFilters", JSONObject());
		DS("musicVolume", 70);
		DS("ambientVolume", 50);
		DS("soundVolume", 100);
		DS("speechVolume", 100);
		DS("keyBinds", JSONArray());
		DS("gamepadBinds", JSONArray());
#undef DS

		width = settings["screenWidth"]->AsInteger();
		height = settings["screenHeight"]->AsInteger();

		//constexpr Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
		//gameLang = opt2lan[settings["language"]->AsInteger()];
		gameLang = Text::GetLangCode(settings["language"]->AsString());

		Audio::MusicVolume = settings["musicVolume"]->AsInteger() / 100.0f;
		Audio::AmbientVolume = settings["ambientVolume"]->AsInteger() / 100.0f;
		Audio::SoundVolume = settings["soundVolume"]->AsInteger() / 100.0f;
		Audio::SpeechVolume = settings["speechVolume"]->AsInteger() / 100.0f;

		botherColliding = settings["botherColliding"]->AsBool();

		auto keyBinds = settings["keyBinds"]->AsArray();
		if (keyBinds.size() != NumKeyBinds)
		{
			keyBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputBindings)
				keyBinds.push_back(new JSONValue(glfwGetKeyScancode(k)));
		}

		auto padBinds = settings["gamepadBinds"]->AsArray();
		if (padBinds.size() != NumKeyBinds)
		{
			padBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputGamepadBindings)
				padBinds.push_back(new JSONValue(k));
		}

		for (int i = 0; i < NumKeyBinds; i++)
		{
			Inputs.Keys[i].ScanCode = keyBinds[i]->AsInteger();
			Inputs.Keys[i].GamepadButton = padBinds[i]->AsInteger();
		}

		doc = VFS::ReadJSON("sound/sounds.json");
		auto sounds = doc->AsObject();
		for (auto category : sounds)
		{
			for (auto sound : category.second->AsObject())
				generalSounds[category.first][sound.first] = std::make_shared<Audio>(sound.second->AsString());
		}
		delete doc;
	}

	void Save()
	{
		settings["screenWidth"] = new JSONValue(width);
		settings["screenHeight"] = new JSONValue(height);

		settings["musicVolume"] = new JSONValue((int)(Audio::MusicVolume * 100.0f));
		settings["ambientVolume"] = new JSONValue((int)(Audio::AmbientVolume * 100.0f));
		settings["soundVolume"] = new JSONValue((int)(Audio::SoundVolume * 100.0f));
		settings["speechVolume"] = new JSONValue((int)(Audio::SpeechVolume * 100.0f));

		settings["botherColliding"] = new JSONValue(botherColliding);

		auto binds = JSONArray();
		for (auto& k : Inputs.Keys)
			binds.push_back(new JSONValue(k.ScanCode));
		settings["keyBinds"] = new JSONValue(binds);

		binds.clear();
		for (auto& k : Inputs.Keys)
			binds.push_back(new JSONValue(k.GamepadButton));
		settings["gamepadBinds"] = new JSONValue(binds);

		try
		{
			VFS::WriteSaveJSON("options.json", new JSONValue(settings));
		}
		catch (std::exception&)
		{
			conprint(2, "Couldn't save settings.");
		}
	}
};

void GameInit()
{
	Audio::Initialize();
	SolBinds::Setup();

	MainCamera = std::make_shared<Camera>();
	musicManager = std::make_shared<MusicManager>();

	ThreadedLoader(Database::LoadGlobalStuff);
	
	town = std::make_shared<Town>();
	dlgBox = std::make_shared<DialogueBox>();
	messager = std::make_shared<Messager>();


#ifdef DEBUG
	//RunTests();
#endif

	//Load the player *here* so we don't get inventory test results mixed in.
	thePlayer.Load();

	cloudImage = std::make_shared<Texture>("altostratus.png");
	starsImage = std::make_shared<Texture>("starfield.png");
	skyImage = std::make_shared<Texture>("skycolors.png");

	if (!LoadLights("lights/initial.json").empty())
	{
		commonUniforms.Lights[0].pos = { 0, 15, 20, 0 };
		commonUniforms.Lights[0].color = { 1, 1, 1, 0.25 };
	}
	if (!LoadCamera("cameras/field.json").empty())
	{
		MainCamera->Set(glm::vec3(0, 0, -6), glm::vec3(0, 110, 0), 60);
	}

	if (town->Villagers.size() == 0)
	{
		town->Villagers.push_back(Database::Find<Villager>("ac:cat01", villagers));
	}
	{
		int i = 0;
		for (auto& vgr : town->Villagers)
		{
			vgr->Manifest();
			vgr->Position = glm::vec3(30, 0, 30 + (i++ * 10));
		}
	}
	thePlayer.Position = glm::vec3(40, 0, 30);

	postFxBuffer = new Framebuffer(Shaders["postfx"], width, height);
	postFxBuffer->SetLut(new Texture("colormap.png"));
}

extern bool skipTitle;

void GameStart(std::vector<TickableP>& tickables)
{
	//Now that we've loaded the key names we can fill in some blanks.
	for (int i = 0; i < NumKeyBinds; i++)
		Inputs.Keys[i].Name = GetKeyName(Inputs.Keys[i].ScanCode);

	tickables.push_back(musicManager);
	tickables.push_back(MainCamera);
	tickables.push_back(town);
	if (skipTitle)
		tickables.push_back(std::make_shared<InGame>());
	else
		tickables.push_back(std::make_shared<TitleScreen>());
}

void GameMouse(double xPosIn, double yPosIn, float xoffset, float yoffset)
{
	xPosIn, yPosIn;
	if (Inputs.MouseHoldMiddle && !MainCamera->Locked)
	{
		auto angles = MainCamera->GetAngles();
		angles.z -= xoffset;
		angles.y -= yoffset;
		MainCamera->Angles(angles);
	}
}

void GameLoopStart()
{
	Audio::Update();
}

void GamePreDraw(float dt)
{
	dt;
	auto pitch = MainCamera->Angles().y;
	if (pitch > 180) pitch -= 360;
	Shaders["sky"]->Set("pitch", pitch);
	cloudImage->Use(1);
	starsImage->Use(2);
	skyImage->Use(3);
}

void GameQuit()
{
	thePlayer.Save();
	town->Save();

}