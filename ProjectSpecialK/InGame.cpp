#include "engine/Utilities.h"
#include "InGame.h"
#include "Messager.h"
#include "Utilities.h"
#include "Camera.h"
#include "Player.h"

extern Tickable root;

InGame::InGame()
{
	dateTimePanel = std::make_shared<DateTimePanel>();
	itemHotbar = std::make_shared<ItemHotbar>();
	iris = std::make_shared<Iris>();

	RemoveAll();
	AddChild(dateTimePanel);
	AddChild(itemHotbar);
	//AddChild(new DialogueBox());
	//AddChild(new Messager);
	//We shouldn't add a DialogueBox or Messager as part of InGame.
	//root.GetChild<T> wouldn't be able to find them.
	AddChild(iris);

	LoadCamera("cameras/field.json");
	root.GetChild<Camera>()->Target(&(thePlayer.Position));
}

bool InGame::Tick(float dt)
{
	Tickable::Tick(dt);

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

