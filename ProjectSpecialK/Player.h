#pragma once
#include "SpecialK.h"

constexpr int MaxOnHand = 7; //40
constexpr int MaxStorage = 5000;
constexpr int StartingOnHand = 20;
constexpr int StartingStorage = 1600;

//Special sentinel for inventory management,
//could mean an item wasn't found or it was nullptr.
constexpr int NoItem = -1;

class Player : public NameableThing, Tickable, public Person
{
private:
	ModelP _model, _hairModel;
	ModelP _topsModel, _bottomsModel, _onePieceModel;
	ModelP _hatModel, _glassesModel, _maskModel, _shoesModel, _bagModel;

	std::array<TextureArray*, 20> Textures;
	std::array<TextureArray*, 32> ClothingTextures;
	
	unsigned char _birthday[2]{ 26, 6 };
	unsigned char _flags[255]{ 0 };

	int findItemSlot(InventoryItemP target);
	int findStorageSlot(InventoryItemP target);

	int eyeStyle{ 0 }, mouthStyle{ 0 }, cheeksStyle{ 2 }, noseStyle{ 1 }, hairStyle{ 0 };
	bool stung{ false };

public:
	std::string Name{ "Mayor" };
	Gender Gender{ Gender::BEnby };

	glm::vec4 SkinTone{ 1.0f, 0.67f, 0.51f, 1.0f };
	glm::vec4 EyeColor{ 0.75f, 0.15f, 0.17f, 1.0f };
	glm::vec4 CheekColor{ 0.87f, 0.48f, 0.29f, 1.0f };
	glm::vec4 HairColor{ 0.73f, 0.11f, 0.31f, 1.0f };

	std::array<InventoryItemP, MaxOnHand> OnHand{ nullptr };
	std::vector<InventoryItemP> Storage;
	int OnHandLimit{ StartingOnHand };
	int StorageLimit{ StartingStorage };
	unsigned int Bells{ 0 };
	
	InventoryItemP HeldTool{ nullptr };
	InventoryItemP Tops{ nullptr };
	InventoryItemP Bottoms{ nullptr };
	InventoryItemP OnePiece{ nullptr };
	InventoryItemP Hat{ nullptr };
	InventoryItemP Glasses{ nullptr };
	InventoryItemP Mask{ nullptr };
	InventoryItemP Socks{ nullptr };
	InventoryItemP Shoes{ nullptr };
	InventoryItemP Bag{ nullptr };

	void LoadModel();
	ModelP Model();
	ModelP Model(int slot);
	std::string Birthday();
	
	void Draw(float dt);
	bool Tick(float dt);

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

	void Save();
	void Load();

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);

	float FindVillagerCollision(glm::vec3 pos);
};

extern Player thePlayer;
