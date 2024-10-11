#include "ItemHotbar.h"
#include "Town.h"

ItemHotbar::ItemHotbar()
{
	for (int i = 0; i < 9; i++)
		RegisterItem(i, -1);

#ifdef DEBUG
	//For testing only until we get further.
	thePlayer.GiveItem(std::make_shared<InventoryItem>("acnh:shovel"));
	thePlayer.GiveItem(std::make_shared<InventoryItem>("psk:toolfallback"));
	RegisterItem(0, 0);
	RegisterItem(2, 1);
#endif

}

void ItemHotbar::Update()
{

}

void ItemHotbar::Tick(float dt)
{
	layout.Tick(dt);
}

void ItemHotbar::Draw(float dt)
{
	layout.Draw(dt);
}

void ItemHotbar::RegisterItem(int slot, int item)
{
	if (slot < 0 || slot > 10)
		return;

	auto panel = layout.GetPanel(fmt::format("hotbarButton_{}", slot));
	auto icon = layout.GetPanel(fmt::format("hotbarIcon_{}", slot));

	if (item == -1) //No item?
	{
		panel->Enabled = false;
		panel->Alpha = 0.5f;
		icon->Text.clear();
	}
	else
	{
		panel->Enabled = true;
		panel->Alpha = 1.0f;
		icon->Text = thePlayer.OnHand[item]->Icon();
	}
}

void ItemHotbar::RegisterItem(int slot, InventoryItemP item)
{
	if (slot < 0 || slot > 10)
		return;

	if (!item)
		RegisterItem(slot, -1);
	for (int i = 0; i < thePlayer.OnHandLimit; i++)
	{
		if (thePlayer.OnHand[i] == item)
			RegisterItem(slot, i);
	}
}
