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

std::shared_ptr<DialogueBox> dlgBox = nullptr;
Audio* bgm = nullptr;
sol::state Sol;

std::shared_ptr<Camera> MainCamera;
std::shared_ptr<Messager> messager;
std::shared_ptr<MusicManager> musicManager;
std::shared_ptr<Town> town;

Framebuffer* postFxBuffer;

std::shared_ptr<Texture> cloudImage, starsImage, skyImage;

void GameInit()
{
	MainCamera = std::make_shared<Camera>();
	musicManager = std::make_shared<MusicManager>();

	ThreadedLoader(Database::LoadGlobalStuff);

	/*
	tickables.push_back(&musicManager);
	tickables.push_back(&MainCamera);
	//auto background = Background("discobg2.png");
	//tickables.push_back(new Background("discobg2.png"));
	//tickables.push_back(new TemporaryTownDrawer());
	auto townDrawer = TemporaryTownDrawer();
	tickables.push_back(new DateTimePanel());
	tickables.push_back(new ItemHotbar());
	//hotbar->Tween(&hotbar->Position.y, -100.0f, 0, 0.002f, glm::bounceEaseOut<float>);
	//hotbar->Tween(&hotbar->Alpha, 0, 0.75f, 0.006f);
	//auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	//auto logoAnim = new PanelLayout(logoJson["cinematic"]);
	//tickables.push_back(logoAnim);
	//tickables.push_back(new DoomMenu());
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);
	auto messager = new Messager();
	tickables.push_back(messager);
	//tickables.push_back(new TextField());
	*/

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

	//rainLayer = new Background("rain.png", glm::vec2(1.0, 2.0));

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
	//thePlayer.Tops = std::make_shared<InventoryItem>("psk:oppai");
	//thePlayer.Bottoms = std::make_shared<InventoryItem>("acnh:denimcutoffs/lightblue");


	postFxBuffer = new Framebuffer(Shaders["postfx"], width, height);
	postFxBuffer->SetLut(new Texture("colormap.png"));

}

extern bool skipTitle;

void GameStart(std::vector<TickableP>& tickables)
{
	tickables.push_back(musicManager);
	tickables.push_back(MainCamera);
	tickables.push_back(town);
	if (skipTitle)
		tickables.push_back(std::make_shared<InGame>());
	else
		tickables.push_back(std::make_shared<TitleScreen>());
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