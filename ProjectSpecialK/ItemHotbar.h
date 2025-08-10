#pragma once

#include "PanelLayout.h"
#include "Item.h"

class ItemHotbar : public Tickable
{
private:
	PanelLayout layout;
	std::array<int, 10> items;

public:
	ItemHotbar();
	void Update();
	bool Tick(float dt);
	void Draw(float dt);
	void Show();
	void Hide();

	void RegisterItem(int slot, int item);
	void RegisterItem(int slot, InventoryItemP item);
};

extern std::shared_ptr<ItemHotbar> itemHotbar;
