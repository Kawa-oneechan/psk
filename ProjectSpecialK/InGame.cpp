#include "InGame.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "DialogueBox.h"
#include "Iris.h"

static Iris* iris;

InGame::InGame()
{
	dateTimePanel = new DateTimePanel();
	itemHotbar = new ItemHotbar();
	iris = new Iris();

	tickables.clear();
	tickables.push_back(dateTimePanel);
	tickables.push_back(itemHotbar);
	tickables.push_back(dlgBox);
	tickables.push_back(iris);
	LoadCamera("cameras/field.json");
	MainCamera.Target(&(thePlayer.Position));
}

InGame::~InGame()
{
	delete iris;
	delete dateTimePanel;
	delete itemHotbar;
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
