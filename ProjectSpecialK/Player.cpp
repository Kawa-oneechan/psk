#include "SpecialK.h"

Player::Player()
{
	_model = nullptr;
	Gender = Gender::BEnby;
	Name = "Mayor";
	_birthday[0] = 0;
	_birthday[1] = 0;
	memset(_flags, 0, sizeof(_flags));
	std::fill(OnHand.begin(), OnHand.end(), nullptr);
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
	return std::any_of(OnHand.cbegin(), OnHand.cend(), [](InventoryItem* i) { return i == nullptr; });
}

bool Player::GiveItem(InventoryItem* item)
{
	auto it = std::find(OnHand.cbegin(), OnHand.cend(), nullptr);
	if (it == OnHand.cend()) return false;
	auto i = std::distance(OnHand.cbegin(), it);
	OnHand[i] = item;
	return true;
}

void Player::SwapItems(int from, int to)
{
	if (from == NO_ITEM || to == NO_ITEM)
		return;
	std::swap(OnHand[from], OnHand[to]);
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

int Player::findStorageSlot(InventoryItem* target)
{
	if (target == nullptr)
		return NO_ITEM;
	for (int i = 0; i < StorageLimit; i++)
	{
		if (Storage[i] == target)
			return i;
	}
	return NO_ITEM;
}

bool Player::Store(int slot)
{
	if (Storage.size() >= StorageLimit)
		return false;
	auto item = OnHand[slot];
	auto storageItem = new InventoryItem(item->AsItem());
	Storage.push_back(storageItem);
	RemoveItem(slot);
	return true;
}
bool Player::Store(InventoryItem* item)
{
	return Store(findItemSlot(item));
}
bool Player::Retrieve(int slot)
{
	if (!HasInventoryRoom())
		return false;
	auto item = Storage[slot];
	auto onHandItem = new InventoryItem(item->AsItem());
	GiveItem(onHandItem);
	Storage.erase(Storage.begin() + slot);
	delete item;
	return true;
}
bool Player::Retrieve(InventoryItem* item)
{
	return Retrieve(findStorageSlot(item));
}

Player thePlayer;
