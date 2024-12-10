#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

class ItemHotbar : public Tickable
{
private:
	PanelLayout layout{ PanelLayout(UI::json["itemhotbar"]) };
	std::array<int, 10> items;

public:
	ItemHotbar();
	void Update();
	void Tick(float dt);
	void Draw(float dt);

	void RegisterItem(int slot, int item);
	void RegisterItem(int slot, InventoryItemP item);
};

extern ItemHotbar* itemHotbar;
