#pragma once
#include "SpecialK.h"

#define MAX_ONHAND 40
#define MAX_STORAGE 5000

//Special sentinel for inventory management,
//could mean an item wasn't found or it was nullptr.
#define NO_ITEM -1

class Player : public NameableThing, Tickable
{
private:
	const Model* _model{ nullptr };
	unsigned char _birthday[2]{ 26, 6 };
	unsigned char _flags[255]{ 0 };

	int findItemSlot(std::shared_ptr<InventoryItem> target);
	int findStorageSlot(std::shared_ptr<InventoryItem> target);

public:
	std::string Name{ "Mayor" };
	Gender Gender{ Gender::BEnby };

	std::array<std::shared_ptr<InventoryItem>, MAX_ONHAND> OnHand{ nullptr };
	std::vector<std::shared_ptr<InventoryItem>> Storage;
	int OnHandLimit{ 20 };
	int StorageLimit{ 1600 };
	unsigned int Bells{ 0 };
	
	std::shared_ptr<InventoryItem> HeldTool{ nullptr };
	std::shared_ptr<InventoryItem> Hat{ nullptr };
	std::shared_ptr<InventoryItem> Glasses{ nullptr };
	std::shared_ptr<InventoryItem> Mask{ nullptr };
	std::shared_ptr<InventoryItem> Outfit{ nullptr };
	std::shared_ptr<InventoryItem> Top{ nullptr };
	std::shared_ptr<InventoryItem> Bottom{ nullptr };
	std::shared_ptr<InventoryItem> Socks{ nullptr };
	std::shared_ptr<InventoryItem> Shoes{ nullptr };
	std::shared_ptr<InventoryItem> Accessory{ nullptr };

	//Player();
	void LoadModel();
	const Model* Model();
	const std::string Birthday();

	//Returns true if the player has room in their inventory according to their current limit.
	bool HasInventoryRoom();
	//Gives the player the specified item. Returns false if there was no room.
	bool GiveItem(std::shared_ptr<InventoryItem> item);
	//Swaps two inventory items. Being position-based, this can swap with empty spots.
	void SwapItems(int from, int to);
	//Swaps two inventory items. Use SwapItems(int, int) if you need to swap with an empty spot.
	void SwapItems(std::shared_ptr<InventoryItem> from, std::shared_ptr<InventoryItem> to);
	//Removes an item from the inventory entirely.
	bool RemoveItem(int slot);
	//Removes an item from the inventory entirely.
	bool RemoveItem(std::shared_ptr<InventoryItem> item);
	//TODO
	bool ConsumeItem(int slot);
	//TODO
	bool ConsumeItem(std::shared_ptr<InventoryItem> item);

	bool Store(int slot);
	bool Store(std::shared_ptr<InventoryItem> item);
	bool Retrieve(int slot);
	bool Retrieve(std::shared_ptr<InventoryItem> item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);
};

extern Player thePlayer;
