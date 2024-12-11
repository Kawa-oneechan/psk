#include "TitleScreen.h"
#include "MusicManager.h"
#include "Background.h"
#include "InputsMap.h"

#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Town.h"
#include "Iris.h"

extern std::vector<Tickable*> tickables;
extern std::vector<Tickable*> newTickables;

TitleScreen::TitleScreen()
{
	tickables.clear();
	tickables.push_back(&musicManager);
	tickables.push_back(&MainCamera);
	//tickables.push_back(new Background("discobg2.png"));
	tickables.push_back(&townDrawer);
	//tickables.push_back(iris);
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	logoAnim = new PanelLayout(logoJson["cinematic"]);

	auto logoJoke = logoAnim->GetPanel("logoJoke");
	//TODO: use JSON
	static std::string lols[] = {
		"100% pure C++!",
		"149% more cats than the other one!",
		"A game within a game within a game!",
		"Absolutely no homestucks. None. Believe us.",
		"An exercise in frustration!",
		"Are you also here to fuck Ankha?",
		"Because fuck you, and your environment!",
		"Catgroovin'!",
		"Cheap imitation!",
		"Datamined!",
		"Eject Hippeux from the island.",
		"FFFFOOOOOOOOOOOOONNNEEEEEE!!!!!!!",
		"Focus tested on Tumblr!",
		"Fox-Hunters' Galop!",
		"From the creators of \"Noxico\"!",
		"From the creators of \"Of Wildcats and Wolves\"!",
		"From the creators of \"The Dating Pool\"!",
		"Hi, Andrea!",
		"Hi, CJ!",
		"Hi, Kate!",
		"Hi, Lancette!",
		"Hi, Letrune!",
		"Hi, Mikael!",
		"Hi, Screwtape!",
		"Hi, Wareya!",
		"It's me.",
		"Made from scratch!",
		"Me meow!",
		"Met de groeten aan Kenney!",
		"Nintendo SWAT team incoming!",
		"NO RESETTING!",
		"No one's around to help.",
		"Not Animal Crossing!",
		"Not powered by Godot!",
		"Not powered by Unity!",
		"Not powered by Unreal!",
		"One-man Devteam!",
		u8"Play as a furry! We won't tell ♥",
		"Powered by FMOD!",
		"Powered by GLFW!",
		"Powered by cola!"
		"Powered by egg!",
		"Powered by spite!",
		"Press \uE0E2 to JSON!",
		"Runs on Doom!",
		"Runs on potato!",
		"Runs on pregnancy test!",
		"Wait, are these guys even paying taxes?",
		"We got EGG",
		"You crack me up, li'l buddy!",
		"Your immigration policy ain't shit, bro.",
		u8"ゆけ! けけライダー",
	};
	logoJoke->Text = lols[rand() % 51];

	LoadCamera("cameras/title.json");
}

void TitleScreen::Tick(float dt)
{
	logoAnim->Tick(dt);
	iris->Tick(dt);

	if (state == State::Init)
	{
		musicManager.Play("title", true);
		state = State::FadeIn;
		iris->In();
	}
	else if (state == State::FadeIn)
	{
		if (iris->Done())
		{
			state = State::Wait;
			logoAnim->Play("open");
		}
	}
	else if (state == State::Wait)
	{
		if (Inputs.KeyDown(Binds::Accept))
		{
			state = State::FadeOut;
			musicManager.FadeOut();
			iris->Out();
		}
	}
	else if (state == State::FadeOut)
	{
		if (iris->Done())
		{
			dead = true;
			dateTimePanel = new DateTimePanel();
			newTickables.push_back(dateTimePanel);
			newTickables.push_back(itemHotbar);
			musicManager.Play(town.Music);
			LoadCamera("cameras/field.json");
			MainCamera.Target(&(town.Villagers[0]->Position));
			iris->In();
		}
	}
}

TitleScreen::~TitleScreen()
{
	delete logoAnim;
}

void TitleScreen::Draw(float dt)
{
	logoAnim->Draw(dt);
	iris->Draw(dt);
}

