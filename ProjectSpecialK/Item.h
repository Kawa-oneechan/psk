#pragma once
#include "engine/Model.h"
#include "engine/JsonUtils.h"
#include "NameableThing.h"

/*
TODO: Bring back the separate Tool, Furniture, and Clothing classes,
but only to handle models and interactions, with a reference back to
the Item that they represent.
*/

class Item : public NameableThing
{
private:
	ModelP iconModel{ nullptr };
	ModelP fieldModel{ nullptr };

public:
	Item(jsonObject& value, const std::string& filename = "");
	int FindVariantByName(const std::string& variantName) const;

	//Returns true if the Item is a Thing, not a Tool, Furniture, or Clothing.
	bool IsThing() const;
	//Returns true if the Item is actually a Tool.
	bool IsTool() const;
	//Returns true if the Item is actually Furniture.
	bool IsFurniture() const;
	//Returns true if the Item is actually Clothing.
	bool IsClothing() const;

	std::vector<std::string> variantNames;

	enum class Type
	{
		Thing,
		Tool,
		Furniture,
		Clothing,
	} Type{ Type::Thing };

	//Price in Bells that this item can be sold for. If it's zero, the item should NOT appear in shops.
	int Price{ 0 };
	//A stack limit of zero is functionally the same as a stack limit of one.
	int StackLimit{ 0 };
	//The item wears down at this many uses. If zero, the item can't wear down.
	int WearLimit{ 0 };

	//The name of the inventory/hotbar icon.
	std::string Icon;

	//If this item is clothing, and specifically a top or onepiece, the base name for the model to be worn by Villagers.
	std::string Style;
	//If this item is clothing, the base name for the model to be worn by Players.
	std::string PlayerModel;

	//The item is allowed to be buried in the ground.
	bool CanBury{ false };
	//The item can be eaten.
	bool CanEat{ false };
	//The item can be placed on the ground as furniture. Should be true only for Items that *are* Furniture.
	bool CanPlace{ false };
	//The item can be placed on either the walls or floor. Only valid for Items that *are* Furniture.
	bool CanGoOnWallsOrFloor{ false };
	//The item can be planted in the ground.
	bool CanPlant{ false };
	//The item can be dropped on the ground.
	bool CanDrop{ false };
	//The item can be given away to other villagers.
	bool CanGive{ false };
	//The item can be sold. Should only be true for Items with non-zero prices.
	bool CanSell{ false };
	//The item can be sold through the drop-off box.
	bool CanDropOff{ false };
	//The item can be put in storage.
	bool CanStore{ false };
	//The item can fit in trash cans.
	bool CanTrash{ false };
	//The item can be a birthday gift.
	bool CanGift{ false };
	//The item can be giftwrapped.
	bool CanWrap{ false };
	//The item can be held in a player's inventory.
	bool CanHave{ false };
	//There would be a canWardrobe, but only Clothing objects would ever appear there -- it's superfluous.

	void DrawFieldIcon(const glm::vec3& position);
	void DrawFieldModel(const glm::vec3& position, float facing);

	enum class FurnKind
	{
		//Things that go on the floor.
		Houseware,
		//(ACNH internal name for Housewares.)
		Floor = FurnKind::Houseware,
		//Things that can go on the floor or on top of supporting Housewares.
		Miscellaneous,
		//(ACNH internal name for Miscellaneous)
		Upper = FurnKind::Miscellaneous,
		//Things that can be placed on walls.
		Wall,
		//Things that can hang from the ceiling.
		Ceiling,
		//Wallpaper.
		RoomWall,
		//Flooring.
		RoomFloor,
		//Things that go on the floor, *under* Housewares.
		Rug,
		//(ACNH internal name for Rugs)
		CeilingRug = FurnKind::Rug,
		//A bug or fish.
		Creature,
	} FurnKind{ FurnKind::Houseware };

	enum class ClothingKind
	{
		Tops,
		Bottoms,
		OnePiece,
		Dress = ClothingKind::OnePiece,
		Hat,
		Cap = ClothingKind::Hat,
		Helmet = ClothingKind::Hat,
		Accessory,
		Socks,
		Shoes,
		Bag,
		Swimwear,
		MarineSuit = ClothingKind::Swimwear,
	} ClothingKind{ ClothingKind::Tops };
};

using ItemP = std::shared_ptr<Item>;

class InventoryItem : public NameableThing
{
private:
	ItemP _wrapped;
	int _data{ 0 };
	int _wear{ 0 };
	int _packaging{ 0 };

public:
	InventoryItem(ItemP wrapped, int variant, int pattern);
	InventoryItem(ItemP wrapped, int data);
	InventoryItem(ItemP wrapped);
	InventoryItem(const std::string& reference);
	InventoryItem(hash hash);
	std::string FullID() const;
	std::string FullName();
	ItemP AsItem() const;

	//Returns true if the wrapped Item is a Thing, not a Tool, Furniture, or Clothing.
	bool IsThing() const;
	//Returns true if the wrapped Item is actually a Tool.
	bool IsTool() const;
	//Returns true if the wrapped Item is actually Furniture.
	bool IsFurniture() const;
	//Returns true if the wrapped Item is actually Clothing.
	bool IsClothing() const;

	std::string Icon() const;
	std::string& Style() const;
	std::string& PlayerModel() const;
	int Data() const;
	void Data(int data);
	int Variant() const;
	void Variant(int variant);
	int Pattern() const;
	void Pattern(int pattern);

	bool InventoryItem::WearDown(int howMuch = 1);

	ItemP Wrapped() const;

	//Safe to delete if true. If not, DO NOT DELETE.
	bool Temporary;
};

using InventoryItemP = std::shared_ptr<InventoryItem>;
