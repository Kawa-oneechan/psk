#include "InGame.h"
#include "Messager.h"
#include "Utilities.h"

InGame::InGame()
{
	dateTimePanel = std::make_shared<DateTimePanel>();
	itemHotbar = std::make_shared<ItemHotbar>();
	iris = std::make_shared<Iris>();

	tickables.clear();
	tickables.push_back(dateTimePanel);
	tickables.push_back(itemHotbar);
	tickables.push_back(dlgBox);
	tickables.push_back(messager);
	tickables.push_back(iris);

	LoadCamera("cameras/field.json");
	MainCamera->Target(&(thePlayer.Position));
}

bool InGame::Tick(float dt)
{
	RevAllTickables(tickables, dt);

	if (state == State::Init)
	{
		state = State::FadeIn;
		iris->In();
	}
	else if (state == State::FadeIn)
	{
		if (iris->Done())
		{
			state = State::Playing;
			itemHotbar->Show();
			dateTimePanel->Show();
		}
	}

	return true;
}

void InGame::Draw(float dt)
{
	DrawAllTickables(tickables, dt);
}
