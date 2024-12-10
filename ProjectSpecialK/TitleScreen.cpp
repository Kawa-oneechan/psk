#include "TitleScreen.h"
#include "MusicManager.h"
#include "Background.h"
#include "InputsMap.h"

#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "Town.h"

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

	if (state == State::Init)
	{
		musicManager.Play("title", true);
		state = State::FadeIn;
		fade = 0.0f;
	}
	else if (state == State::FadeIn)
	{
		fade += dt;
		if (fade > 1.0f)
		{
			state = State::Wait;
			fade = 1.0f;
			logoAnim->Play("open");
		}
	}
	else if (state == State::Wait)
	{
		if (Inputs.KeyDown(Binds::Accept))
		{
			state = State::FadeOut;
			fade = 0.0f;
			musicManager.FadeOut();
		}
	}
	else if (state == State::FadeOut)
	{
		fade += dt;
		if (fade > 1.0f)
		{
			dead = true;
			dateTimePanel = new DateTimePanel();
			newTickables.push_back(dateTimePanel);
			newTickables.push_back(itemHotbar);
			musicManager.Play(town.Music);
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
	Sprite::DrawText(fmt::format("TitleScreen::fade = {}", fade), glm::vec2(4));
}

