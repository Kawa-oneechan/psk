#pragma once
#include "SpecialK.h"

/*
Body texture layout:
 0 -  2. Body (alb, mix, nrm)
 3 -  5. Nose
 6 -  8. Cheek (three albedo layers/options)
 9 - 11. Eyes (26 options, 16 layers, regular)
12 - 14. Eyes (stung)
15 - 17. Mouth (4 options, 9 layers)

Clothing texture layout:
 0 -  3. Tops (alb, mix, nrm, op) OR Onepiece
 4 -  7. Bottoms
 8 - 11. Shoes
12 - 15. Hat
16 - 19. Glasses
20 - 23. Glasses alpha
24 - 27. Accessories
28 - 31. Bag
*/

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
	ModelP _model, _hairModel;
	ModelP _topsModel, _bottomsModel, _onePieceModel;
	ModelP _hatModel, _glassesModel, _maskModel, _shoesModel, _bagModel;

	std::array<Texture*, 20> Textures;
	std::array<Texture*, 32> ClothingTextures;
	glm::vec4 skinColor, hairColor, eyeColor;
	
	unsigned char _birthday[2]{ 26, 6 };
	unsigned char _flags[255]{ 0 };

	int findItemSlot(InventoryItemP target);
	int findStorageSlot(InventoryItemP target);

	int face{ 0 }, mouth{ 0 };
	bool stung{ false };

public:
	std::string Name{ "Mayor" };
	Gender Gender{ Gender::BEnby };

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

	void SetFace(int face);
	void SetMouth(int mouth);

	void Draw(double dt);

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
