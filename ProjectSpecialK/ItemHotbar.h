#pragma once

#include "PanelLayout.h"
#include "Item.h"

class ItemHotbar : public Tickable
{
private:
	std::shared_ptr<PanelLayout> layout;
	std::array<int, 10> items{};

public:
	ItemHotbar();
	void Update();
	bool Tick(float dt) override;
	void Draw(float dt) override;
	void Show();
	void Hide();

	void RegisterItem(int slot, int item);
	void RegisterItem(int slot, InventoryItemP item);
};

extern std::shared_ptr<ItemHotbar> itemHotbar;
