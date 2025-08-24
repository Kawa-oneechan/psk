#include <filesystem>
#include "engine/Text.h"
#include "engine/Framebuffer.h"
#include "engine/Console.h"
#include "Types.h"
#include "Game.h"
#include "DialogueBox.h"
#include "Camera.h"
#include "Messager.h"
#include "MusicManager.h"
#include "Town.h"
#include "InGame.h"
#include "TitleScreen.h"
#include "Utilities.h"
#include "Database.h"
#include "Player.h"

#ifdef _MSC_VER
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

#ifdef DEBUG
extern void RunTests();
#endif

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;

std::shared_ptr<DialogueBox> dlgBox = nullptr;
Audio* bgm = nullptr;

std::shared_ptr<Camera> MainCamera;
std::shared_ptr<Messager> messager;
std::shared_ptr<MusicManager> musicManager;
std::shared_ptr<Town> town;

Framebuffer* postFxBuffer;

std::shared_ptr<Texture> cloudImage, starsImage, skyImage;

extern bool botherColliding;
bool showPos = false;

namespace SolBinds
{
	extern void Setup();
}

namespace UI
{
	std::map<std::string, glm::vec4> themeColors;
	std::vector<glm::vec4> textColors;

	std::shared_ptr<Texture> controls{ nullptr };

	jsonValue json;
	jsonValue settings;

	std::string initFile = "init.json";

	void Load()
	{
		UI::json = VFS::ReadJSON("ui/ui.json");
		if (!UI::json)
			FatalError("Could not read ui/ui.json. Something is very wrong.");
		auto json = UI::json.as_object();
		auto colors = json["colors"].as_object();
		for (auto& ink : colors["theme"].as_object())
		{
			themeColors[ink.first] = GetJSONColor(ink.second);
		}
		for (auto& ink : colors["text"].as_array())
		{
			textColors.push_back(GetJSONColor(ink));
		}

		try
		{
			UI::settings = VFS::ReadSaveJSON("options.json");
		}
		catch (std::runtime_error&)
		{
			UI::settings = json5pp::parse5("{}");
		}

		auto settings = UI::settings.as_object();

#define DS(K, V) if (!settings[K]) settings[K] = jsonValue(V)
#define DA(K, V) if (!settings[K]) settings[K] = json5pp::array(V)
#define DO(K, V) if (!settings[K]) settings[K] = json5pp::object(V)
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
		DO("contentFilters", {});
		DS("musicVolume", 70);
		DS("ambientVolume", 50);
		DS("soundVolume", 100);
		DS("speechVolume", 100);
		DA("keyBinds", {});
		DA("gamepadBinds", {});
#undef DO
#undef DA
#undef DS

		width = settings["screenWidth"].as_integer();
		height = settings["screenHeight"].as_integer();

		//constexpr Language opt2lan[] = { Language::USen, Language::JPja, Language::EUde, Language::EUes, Language::EUfr, Language::EUit, Language::EUhu, Language::EUnl, Language::EUen };
		//gameLang = opt2lan[settings["language"]->AsInteger()];
		gameLang = Text::GetLangCode(settings["language"].as_string());

		Audio::MusicVolume = settings["musicVolume"].as_integer() / 100.0f;
		Audio::AmbientVolume = settings["ambientVolume"].as_integer() / 100.0f;
		Audio::SoundVolume = settings["soundVolume"].as_integer() / 100.0f;
		Audio::SpeechVolume = settings["speechVolume"].as_integer() / 100.0f;

		botherColliding = settings["botherColliding"].as_boolean();

		auto keyBinds = settings["keyBinds"].as_array();
		if (keyBinds.size() != NumKeyBinds)
		{
			keyBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputBindings)
				keyBinds.push_back(glfwGetKeyScancode(k));
		}

		auto padBinds = settings["gamepadBinds"].as_array();
		if (padBinds.size() != NumKeyBinds)
		{
			padBinds.reserve(NumKeyBinds);
			for (auto &k : DefaultInputGamepadBindings)
				padBinds.push_back(k);
		}

		for (int i = 0; i < NumKeyBinds; i++)
		{
			Inputs.Keys[i].ScanCode = keyBinds[i].as_integer();
			Inputs.Keys[i].GamepadButton = padBinds[i].as_integer();
		}

		auto sounds = VFS::ReadJSON("sound/sounds.json").as_object();
		for (auto category : sounds)
		{
			for (auto sound : category.second.as_object())
				generalSounds[category.first][sound.first] = std::make_shared<Audio>(sound.second.as_string());
		}
	}

	void Save()
	{
		auto settings = UI::settings.as_object();
		settings["screenWidth"] = width;
		settings["screenHeight"] = height;

		settings["musicVolume"] = (int)(Audio::MusicVolume * 100.0f);
		settings["ambientVolume"] = (int)(Audio::AmbientVolume * 100.0f);
		settings["soundVolume"] = (int)(Audio::SoundVolume * 100.0f);
		settings["speechVolume"] = (int)(Audio::SpeechVolume * 100.0f);

		settings["botherColliding"] = botherColliding;

		auto binds = json5pp::array({});
		for (auto& k : Inputs.Keys)
			binds.as_array().push_back(k.ScanCode);
		settings["keyBinds"] = std::move(binds);

		auto binds2 = json5pp::array({});
		for (auto& k : Inputs.Keys)
			binds2.as_array().push_back(k.GamepadButton);
		settings["gamepadBinds"] = std::move(binds2);

		try
		{
			VFS::WriteSaveJSON("options.json", UI::settings);
		}
		catch (std::exception&)
		{
			conprint(2, "Couldn't save settings.");
		}
	}
};

void GameInit()
{
	SolBinds::Setup();

	MainCamera = std::make_shared<Camera>();
	musicManager = std::make_shared<MusicManager>();

	ThreadedLoader(Database::LoadGlobalStuff);

	town = std::make_shared<Town>();
	dlgBox = std::make_shared<DialogueBox>();
	messager = std::make_shared<Messager>();

	UI::controls = std::make_shared<Texture>("ui/controls.png");

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

	commonUniforms.GrassColor = 0.5f;
}

void GamePrepSaveDirs(const fs::path& savePath)
{
	fs::create_directory(savePath / "villagers");
	fs::create_directory(savePath / "map");
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

void GameResize()
{}

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

void GamePostDraw(float dt)
{
	dt;
	if (showPos)
	{
		Sprite::DrawText(
			fmt::format(
				"{}\npos: {}, {}, {}",
				thePlayer.Name,
				thePlayer.Position.x, thePlayer.Position.y, thePlayer.Position.z),
			glm::vec2(0)
			);
	}
}

void GameQuit()
{
	thePlayer.Save();
	town->Save();
}
