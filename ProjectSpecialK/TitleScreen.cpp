#include "TitleScreen.h"
#include "MusicManager.h"
#include "Background.h"
#include "InputsMap.h"

#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "DialogueBox.h"
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
	tickables.push_back(&town);
	//tickables.push_back(iris);
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	logoAnim = new PanelLayout(logoJson["cinematic"]);

	{
		auto logoJoke = logoAnim->GetPanel("logoJoke");
		int options = 0;
		for (int i = 0; i < 100; i++)
		{
			auto result = Text::Get(fmt::format("logojoke:{}", i));
			if (result.length() >= 3 && result.substr(0, 3) == "???")
			{
				options = i;
				break;
			}
		}
		if (options == 0)
		{
			conprint(2, "TitleScreen: could not find a logo joke.");
			logoJoke->Text = "???";
		}
		else
		{
			int choice = std::rand() % options;
			logoJoke->Text = Text::Get(fmt::format("logojoke:{}", choice));
		}
		Text::Forget("logojoke:");
	}
	
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
			newTickables.push_back(dlgBox);
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

