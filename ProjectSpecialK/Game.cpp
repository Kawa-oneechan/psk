#include <filesystem>
#include "engine/Text.h"
#include "engine/Framebuffer.h"
#include "engine/Console.h"
#include "engine/Game.h"
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

#ifdef DEBUG
extern void RunTests();
#endif

constexpr int ScreenWidth = 1920;
constexpr int ScreenHeight = 1080;

Audio* bgm = nullptr;
std::map<std::string, std::map<std::string, std::shared_ptr<Audio>>> generalSounds;

extern std::shared_ptr<TextureArray> cloudImage;
extern std::shared_ptr<Texture> starsImage, skyImage;

Framebuffer* postFxBuffer;

extern bool botherColliding;
bool showPos = false;

namespace SolBinds
{
	extern void Setup(sol::state& sol);
}

namespace UI
{
	std::shared_ptr<Texture> controls;
}

void Game::LoadSettings(jsonObject& settings)
{
#define DS(K, V) if (!settings[K]) settings[K] = jsonValue(V)
#define DA(K, V) if (!settings[K]) settings[K] = json5pp::array(V)
#define DO(K, V) if (!settings[K]) settings[K] = json5pp::object(V)
	DS("continue", (int)LoadSpawnChoice::FrontDoor);
	DS("speech", (int)DialogueBox::Sound::Bebebese);
	DS("pingRate", 3);
	DS("balloonChance", 15);
	DS("cursorScale", 100);
	DS("botherColliding", true);
	DS("24hour", true);
	DO("contentFilters", {});
#undef DO
#undef DA
#undef DS

	botherColliding = settings["botherColliding"].as_boolean();
}

void Game::SaveSettings(jsonObject& settings)
{
	settings["botherColliding"] = botherColliding;
}

void Game::Initialize()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);

	SolBinds::Setup(Sol);


	commonUniforms.Fresnel = true;

	ThreadedLoader(Database::LoadGlobalStuff);


	UI::controls = std::make_shared<Texture>("ui/controls.png");


	auto testScript = R"SOL(

	function start()
		-- local total = 2
		-- dialogue("That's <bells:total>.")
		dialogue("Holy shit! <cap><a><item:4142720200>?")
	end

	-- start();

	)SOL";
	Sol.do_string(testScript);


	{
		auto sounds = VFS::ReadJSON("sound/sounds.json").as_object();
		for (auto category : sounds)
		{
			for (auto sound : category.second.as_object())
				generalSounds[category.first][sound.first] = std::make_shared<Audio>(sound.second.as_string());
		}
	}

	//Load the player *here* so we don't get inventory test results mixed in.
	thePlayer.Load();

	cloudImage = std::make_shared<TextureArray>("sky/clouds*.png");
	starsImage = std::make_shared<Texture>("sky/starfield.png");
	skyImage = std::make_shared<Texture>("sky/skycolors.png");

	if (!LoadLights("lights/initial.json").empty())
	{
		commonUniforms.Lights[0].pos = { 0, 15, 20, 0 };
		commonUniforms.Lights[0].color = { 1, 1, 1, 0.25 };
	}
	if (!LoadCamera("cameras/field.json").empty())
	{
		root.GetChild<Camera>()->Set(glm::vec3(0, 0, -6), glm::vec3(0, 110, 0), 60);
	}

	postFxBuffer = new Framebuffer(Shaders["postfx"], width, height);
	postFxBuffer->SetLut(new TextureArray("colormap*.png"));

	//commonUniforms.GrassColor = 0.5f;
}

void Game::PrepareSaveDirs()
{
	VFS::MakeSaveDir("villagers");
	VFS::MakeSaveDir("map");
}

extern bool skipTitle, wireframe;

void Game::Start(Tickable& root)
{
	//Now that we've loaded the key names we can fill in some blanks.
	for (int i = 0; i < NumKeyBinds; i++)
		Inputs.Keys[i].Name = GetKeyName(Inputs.Keys[i].ScanCode);

	root.AddChild(new MusicManager());
	root.AddChild(new Camera());
	root.AddChild(new Town());
	if (skipTitle)
		root.AddChild(new InGame());
	else
		root.AddChild(new TitleScreen());
	root.AddChild(new DialogueBox());

	auto town = root.GetChild<Town>();
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

#ifdef DEBUG
	RunTests();
#endif
}

void Game::OnKey(int key, int scancode, int action, int mods)
{
	if (scancode == Inputs.Keys[(int)Binds::Screenshot].ScanCode && action == 1)
	{
		Screenshot();
		return;
	}

	if (key == GLFW_KEY_F1 && action == 1)
	{
		wireframe = !wireframe;
		return;
	}
}

void Game::OnMouse(double xPosIn, double yPosIn, float xoffset, float yoffset)
{
	xPosIn, yPosIn;
	static Camera* camera = nullptr;
	if (!camera)
		camera = root.GetChild<Camera>();
	if (!camera)
		return;

	if (Inputs.MouseHoldMiddle && !camera->Locked)
	{
		auto angles = camera->GetAngles();
		angles.z -= xoffset;
		angles.y -= yoffset;
		camera->Angles(angles);
	}
}

void Game::OnResize()
{}

void Game::LoopStart()
{
}

void Game::PreDraw(float dt)
{
	(void)(dt);
}

void Game::PostDraw(float dt)
{
	(void)(dt);
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

void Game::OnQuit()
{
	thePlayer.Save();
	root.GetChild<Town>()->Save();
}
