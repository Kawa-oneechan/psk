#include "SpecialK.h"

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

int Player::findItemSlot(std::shared_ptr<InventoryItem> target)
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
	return std::any_of(OnHand.cbegin(), OnHand.cend(), [](std::shared_ptr<InventoryItem> i) { return i == nullptr; });
}

bool Player::GiveItem(std::shared_ptr<InventoryItem> item)
{
	auto it = std::find(OnHand.cbegin(), OnHand.cend(), nullptr);
	if (it == OnHand.cend()) return false;
	auto i = std::distance(OnHand.cbegin(), it);
	OnHand[i] = std::move(item);
	return true;
}

void Player::SwapItems(int from, int to)
{
	if (from == NO_ITEM || to == NO_ITEM)
		return;
	std::swap(OnHand[from], OnHand[to]);
}

void Player::SwapItems(std::shared_ptr<InventoryItem> from, std::shared_ptr<InventoryItem> to)
{
	SwapItems(findItemSlot(from), findItemSlot(to));
}

bool Player::RemoveItem(int slot)
{
	if (slot == NO_ITEM)
		return false;
	if (OnHand[slot] == nullptr)
		return false;
	OnHand[slot] = nullptr;
	return true;
}

bool Player::RemoveItem(std::shared_ptr<InventoryItem> item)
{
	return RemoveItem(findItemSlot(item));
}

bool Player::ConsumeItem(int slot)
{
	if (slot == NO_ITEM)
		return false;
	return false;
}

bool Player::ConsumeItem(std::shared_ptr<InventoryItem> item)
{
	return ConsumeItem(findItemSlot(item));
}

int Player::findStorageSlot(std::shared_ptr<InventoryItem> target)
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
	Storage.push_back(OnHand[slot]);
	RemoveItem(slot);
	return true;
}

bool Player::Store(std::shared_ptr<InventoryItem> item)
{
	return Store(findItemSlot(item));
}

bool Player::Retrieve(int slot)
{
	if (!HasInventoryRoom())
		return false;
	GiveItem(Storage[slot]);
	Storage.erase(Storage.begin() + slot);
	return true;
}

bool Player::Retrieve(std::shared_ptr<InventoryItem> item)
{
	return Retrieve(findStorageSlot(item));
}

Player thePlayer;
