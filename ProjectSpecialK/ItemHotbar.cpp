#include "engine/InputsMap.h"
#include "ItemHotbar.h"
#include "Player.h"
#include "Town.h"
#include "Messager.h"
#include "Utilities.h"
#include "Database.h"

ItemHotbar::ItemHotbar()
{
	auto json = VFS::ReadJSON("ui/itemhotbar.json").as_object();
	layout = std::make_shared<PanelLayout>(json["itemhotbar"]);
	layout->onClick = [](const std::string& id) {
		root.GetChild<Messager>()->Add(fmt::format("Hotbar: {}", id));
	};

	for (int i = 0; i < 10; i++)
		RegisterItem(i, -1);

#ifdef DEBUG
	//For testing only until we get further.
	RegisterItem(0, 0);
	RegisterItem(2, 1);
#endif

	Update();
}

void ItemHotbar::Update()
{
	for (int i = 0; i < 9; i++)
	{
		auto panel = layout->GetPanel(fmt::format("hotbarNumber_{}", i));
		panel->Text = Inputs.Keys[((int)Binds::HotBar1) + i].Name;
	}
}

bool ItemHotbar::Tick(float dt)
{
	//TODO: respond to clicks
	return layout->Tick(dt);
}

void ItemHotbar::Draw(float dt)
{
	layout->Draw(dt);
}

void ItemHotbar::Show()
{
	if (layout->Playing()) return;
	layout->Play("show");
}

void ItemHotbar::Hide()
{
	if (layout->Playing()) return;
	layout->Play("hide");
}

void ItemHotbar::RegisterItem(int slot, int item)
{
	if (slot < 0 || slot > 10)
		return;

	auto panel = layout->GetPanel(fmt::format("hotbarButton_{}", slot));
	auto icon = layout->GetPanel(fmt::format("hotbarIcon_{}", slot));

	if (item == -1) //No item?
	{
		panel->Enabled = false;
		panel->Alpha = 0.5f;
		icon->Alpha = 0.0f;
	}
	else
	{
		panel->Enabled = true;
		panel->Alpha = 1.0f;
		icon->Texture = Database::ItemIcons.get();
		icon->SetFrame(thePlayer.OnHand[item]->Icon());
		icon->Alpha = 1.0f;
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
