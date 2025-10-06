#pragma once

#include <memory>
#include "engine/Tickable.h"
#include "DateTimePanel.h"
#include "ItemHotbar.h"
#include "DialogueBox.h"
#include "Iris.h"

class InGame : public Tickable
{
private:
	enum class State
	{
		Init, FadeIn, Playing
	} state{ State::Init };

	std::shared_ptr<Iris> iris;
	std::shared_ptr<ItemHotbar> itemHotbar;
	std::shared_ptr<DateTimePanel> dateTimePanel;
	
public:
	InGame();

	bool Tick(float dt) override;
};
