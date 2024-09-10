#pragma once
#include "SpecialK.h"

constexpr int MaxOnHand = 7; //40
constexpr int MaxStorage = 5000;
constexpr int StartingOnHand = 20;
constexpr int StartingStorage = 1600;

//Special sentinel for inventory management,
//could mean an item wasn't found or it was nullptr.
constexpr int NoItem = -1;

class Player : public NameableThing, Tickable
{
private:
	ModelP _model{ nullptr };
	unsigned char _birthday[2]{ 26, 6 };
	unsigned char _flags[255]{ 0 };

	int findItemSlot(InventoryItemP target);
	int findStorageSlot(InventoryItemP target);

public:
	std::string Name{ "Mayor" };
	Gender Gender{ Gender::BEnby };

	std::array<InventoryItemP, MaxOnHand> OnHand{ nullptr };
	std::vector<InventoryItemP> Storage;
	int OnHandLimit{ StartingOnHand };
	int StorageLimit{ StartingStorage };
	unsigned int Bells{ 0 };
	
	InventoryItemP HeldTool{ nullptr };
	InventoryItemP Hat{ nullptr };
	InventoryItemP Glasses{ nullptr };
	InventoryItemP Mask{ nullptr };
	InventoryItemP Clothing{ nullptr };
	InventoryItemP Top{ nullptr };
	InventoryItemP Bottom{ nullptr };
	InventoryItemP Socks{ nullptr };
	InventoryItemP Shoes{ nullptr };
	InventoryItemP Accessory{ nullptr };

	void LoadModel();
	ModelP Model();
	std::string Birthday();

	//Returns true if the player has room in their inventory according to their current limit.
	bool HasInventoryRoom();
	//Gives the player the specified item. Returns false if there was no room.
	bool GiveItem(InventoryItemP item);
	//Swaps two inventory items. Being position-based, this can swap with empty spots.
	void SwapItems(int from, int to);
	//Swaps two inventory items. Use SwapItems(int, int) if you need to swap with an empty spot.
	void SwapItems(InventoryItemP from, InventoryItemP to);
	//Removes an item from the inventory entirely.
	bool RemoveItem(int slot);
	//Removes an item from the inventory entirely.
	bool RemoveItem(InventoryItemP item);
	//TODO
	bool ConsumeItem(int slot);
	//TODO
	bool ConsumeItem(InventoryItemP item);

	bool Store(int slot);
	bool Store(InventoryItemP item);
	bool Retrieve(int slot);
	bool Retrieve(InventoryItemP item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);
};

extern Player thePlayer;
