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
	//tickables.push_back(new Background("discobg2.png"));
	tickables.push_back(&townDrawer);
	//tickables.push_back(iris);
	auto logoJson = VFS::ReadJSON("cinematics/logo/logo.json")->AsObject();
	logoAnim = new PanelLayout(logoJson["cinematic"]);
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

