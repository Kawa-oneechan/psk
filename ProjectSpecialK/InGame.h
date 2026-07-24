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

	std::shared_ptr<Iris> iris{ std::make_shared<Iris>() };
	std::shared_ptr<ItemHotbar> itemHotbar{ std::make_shared<ItemHotbar>() };
	std::shared_ptr<DateTimePanel> dateTimePanel{ std::make_shared<DateTimePanel>() };
	
public:
	InGame();

	bool Tick(float dt) override;
};
