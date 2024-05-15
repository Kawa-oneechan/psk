#include "SpecialK.h"

Player::Player()
{
	_model = nullptr;
	Gender = Gender::BEnby;
	Name = "Mayor";
	_birthday[0] = 0;
	_birthday[1] = 0;
	memset(_flags, 0, sizeof(_flags));
	memset(OnHand, 0, sizeof(OnHand));
	Storage.clear();
	OnHandLimit = 20;
	StorageLimit = 1600;
	Bells = 0;
	HeldTool = nullptr;
	Hat = nullptr;
	Glasses = nullptr;
	Mask = nullptr;
	Outfit = nullptr;
	Top = nullptr;
	Bottom = nullptr;
	Socks = nullptr;
	Shoes = nullptr;
	Accessory = nullptr;
}

void Player::LoadModel()
{

}

const Model* Player::Model()
{
	if (_model == nullptr)
		LoadModel();
	return _model;
}

const std::string Player::Birthday()
{
	return TextDateMD(_birthday[1], _birthday[0]);
}

int Player::findItemSlot(InventoryItem* target)
{
	if (target == nullptr)
		return NO_ITEM;
	for (int i = 0; i < OnHandLimit; i++)
	{
		if (OnHand[i] == target)
			return i;
	}
	return NO_ITEM;
}

bool Player::HasInventoryRoom()
{
	for (int i = 0; i < OnHandLimit; i++)
	{
		if (OnHand[i] == nullptr)
			return true;
	}
	return false;
}

bool Player::GiveItem(InventoryItem* item)
{
	if (!HasInventoryRoom())
		return false;
	for (int i = 0; i < OnHandLimit; i++)
	{
		if (OnHand[i] == nullptr)
		{
			OnHand[i] = item;
			return true;
		}
	}
	//Somehow, Palpatine has returned.
	return false;
}

void Player::SwapItems(int from, int to)
{
	if (from == NO_ITEM || to == NO_ITEM)
		return;

	auto temp = OnHand[to];
	OnHand[to] = OnHand[from];
	OnHand[from] = temp;
}

void Player::SwapItems(InventoryItem* from, InventoryItem* to)
{
	SwapItems(findItemSlot(from), findItemSlot(to));
}

bool Player::RemoveItem(int slot)
{
	if (slot == NO_ITEM)
		return false;
	if (OnHand[slot] == nullptr)
		return false;
	delete OnHand[slot];
	OnHand[slot] = nullptr;
	return true;
}

bool Player::RemoveItem(InventoryItem* item)
{
	return RemoveItem(findItemSlot(item));
}

bool Player::ConsumeItem(int slot)
{
	if (slot == NO_ITEM)
		return false;
	return false;
}

bool Player::ConsumeItem(InventoryItem* item)
{
	return ConsumeItem(findItemSlot(item));
}


Player thePlayer;
