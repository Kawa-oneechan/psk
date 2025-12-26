#pragma once
#include "Villager.h"

constexpr int MaxOnHand = 7; //40
constexpr int MaxStorage = 5000;
constexpr int StartingOnHand = 20;
constexpr int StartingStorage = 1600;

//Special sentinel for inventory management,
//could mean an item wasn't found or it was nullptr.
constexpr int NoItem = -1;

class Player : public NameableThing, public Person
{
private:
	ModelP _hairModel;

	unsigned char _flags[255]{ 0 };

	int findItemSlot(InventoryItemP target);
	int findStorageSlot(InventoryItemP target);

	int eyeStyle{ 0 }, mouthStyle{ 0 }, cheeksStyle{ 2 }, noseStyle{ 1 }, hairStyle{ 0 };
	bool stung{ false };

public:
	std::string Name{ "Mayor" };

	glm::vec4 SkinTone{ 0.94f, 0.52f, 0.33f, 1.0f };
	glm::vec4 SkinEdge{ 0.7f, 0.0f, 0.09f, 1.0f };
	glm::vec4 EyeColor{ 0.03f, 0.03f, 0.03f, 1.0f };
	glm::vec4 CheekColor{ 1.0f, 0.82f, 0.84f, 1.0f };
	glm::vec4 HairColor{ 0.022f, 0.022f, 0.022f, 1.0f };
	glm::vec4 HairHiliteColor{ 0.06f, 0.06f, 0.06f, 1.0f };
	

	std::array<InventoryItemP, MaxOnHand> OnHand{ nullptr };
	std::vector<InventoryItemP> Storage;
	int OnHandLimit{ StartingOnHand };
	int StorageLimit{ StartingStorage };
	unsigned int Bells{ 0 };

	void LoadModel();
	ModelP Model();
	std::string Birthday();
	
	void Draw(float dt) override;
	bool Tick(float dt) override;

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
	//Takes one item from a stack. If it's not stackable or there was only one item, removes it.
	bool ConsumeItem(int slot);
	//Takes one item from a stack. If it's not stackable or there was only one item, removes it.
	bool ConsumeItem(InventoryItemP item);

	//Moves the specified inventory item from the player's inventory to their storage.
	bool Store(int slot);
	//Moves the specified inventory item from the player's inventory to their storage.
	bool Store(InventoryItemP item);
	//Moves the specified inventory item from the player's storage to their inventory.
	bool Retrieve(int slot);
	//Moves the specified inventory item from the player's storage to their inventory.
	bool Retrieve(InventoryItemP item);

	//Wears an item. Returns false if the item is not clothing or there is no space to put what is already worn.
	bool Wear(int slot);
	//Wears an item. Returns false if the item is not clothing or there is no space to put what is already worn.
	bool Wear(InventoryItemP item);

	void Save();
	void Load();

	void Serialize(jsonValue& target);
	void Deserialize(jsonValue& source);
};

extern Player thePlayer;
