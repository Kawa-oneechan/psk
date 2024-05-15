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
	const Model* _model;
	unsigned char _birthday[2];
	unsigned char _flags[255];

	int findItemSlot(InventoryItem* target);

public:
	std::string Name;
	Gender Gender;

	InventoryItem* OnHand[MAX_ONHAND];
	std::vector<InventoryItem*> Storage;
	int OnHandLimit;
	int StorageLimit;
	unsigned int Bells;
	
	InventoryItem* HeldTool;
	InventoryItem* Hat;
	InventoryItem* Glasses;
	InventoryItem* Mask;
	InventoryItem* Outfit;
	InventoryItem* Top;
	InventoryItem* Bottom;
	InventoryItem* Socks;
	InventoryItem* Shoes;
	InventoryItem* Accessory;

	Player();
	void LoadModel();
	const Model* Model();
	const std::string Birthday();

	//Returns true if the player has room in their inventory according to their current limit.
	bool HasInventoryRoom();
	//Gives the player the specified item. Returns false if there was no room.
	bool GiveItem(InventoryItem* item);
	//Swaps two inventory items. Being position-based, this can swap with empty spots.
	void SwapItems(int from, int to);
	//Swaps two inventory items. Use SwapItems(int, int) if you need to swap with an empty spot.
	void SwapItems(InventoryItem* from, InventoryItem* to);
	//Removes an item from the inventory entirely.
	bool RemoveItem(int slot);
	//Removes an item from the inventory entirely.
	bool RemoveItem(InventoryItem* item);
	//TODO
	bool ConsumeItem(int slot);
	//TODO
	bool ConsumeItem(InventoryItem* item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);
};

extern Player thePlayer;
