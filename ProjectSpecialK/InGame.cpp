#include "engine/Utilities.h"
#include "InGame.h"
#include "Messager.h"
#include "Utilities.h"
#include "Camera.h"
#include "Player.h"

InGame::InGame()
{
	dateTimePanel = std::make_shared<DateTimePanel>();
	itemHotbar = std::make_shared<ItemHotbar>();
	iris = std::make_shared<Iris>();

	ChildTickables.clear();
	ChildTickables.push_back(dateTimePanel);
	ChildTickables.push_back(itemHotbar);
	ChildTickables.push_back(dlgBox);
	ChildTickables.push_back(messager);
	ChildTickables.push_back(iris);

	LoadCamera("cameras/field.json");
	MainCamera->Target(&(thePlayer.Position));
}

bool InGame::Tick(float dt)
{
	RevAllTickables(ChildTickables, dt);

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
	DrawAllTickables(ChildTickables, dt);
}
