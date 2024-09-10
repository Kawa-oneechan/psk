#pragma once

#include "Model.h"

class Item;
class Tool;
class Furniture;
class Clothing;

using ItemP = std::shared_ptr<Item>;
using ToolP = std::shared_ptr<Tool>;
using FurnitureP = std::shared_ptr<Furniture>;
using ClothingP = std::shared_ptr<Clothing>;

//An Item, aka a Thing, is something that cannot serve as furniture, cannot be held in hand, and cannot be worn. It serves as a crafting ingredient at best, a MacGuffin at worst.
class Item : public NameableThing
{
private:
	ModelP _model;
public:
	Item(JSONObject& value, const std::string& filename = "");
	int FindVariantByName(const std::string& variantName) const;
	std::vector<std::string> variantNames;

	enum class Type
	{
		Thing,
		Tool,
		Furniture,
		Clothing,
	} Type{ Type::Thing };

	//Price in Bells that this item can be sold for. If it's zero, the item should NOT appear in shops.
	int price{ 0 };
	//A stack limit of zero is functionally the same as a stack limit of one.
	int stackLimit{ 0 };

	//The item is allowed to be buried in the ground.
	bool canBury{ false };
	//The item can be eaten.
	bool canEat{ false };
	//The item can be placed on the ground as furniture. Should be true only for Items that *are* Furniture.
	bool canPlace{ false };
	//The item can be planted in the ground.
	bool canPlant{ false };
	//The item can be dropped on the ground.
	bool canDrop{ false };
	//The item can be given away to other villagers.
	bool canGive{ false };
	//The item can be sold. Should only be true for Items with non-zero prices.
	bool canSell{ false };
	//The item can be sold through the drop-off box.
	bool canDropOff{ false };
	//The item can be put in storage.
	bool canStore{ false };
	//The item can fit in trash cans.
	bool canTrash{ false };
	//The item can be a birthday gift.
	bool canGift{ false };
	//The item can be giftwrapped.
	bool canWrap{ false };
	//There would be a canWardrobe, but only Clothing objects would ever appear there -- it's superfluous.

	//Returns true if the Item is in fact an Item (a Thing), not a Tool, Furniture, or Clothing.
	bool IsItem() const;
	//Returns true if the Item is actually a Tool.
	bool IsTool() const;
	//Returns true if the Item is actually Furniture.
	bool IsFurniture() const;
	//Returns true if the Item is actually Clothing.
	bool IsClothing() const;
	//Returns the Item itself. Why? Because I can.
	ItemP AsItem() const;
	//Returns the Item as a Tool. Should only be called if IsTool returns true.
	ToolP AsTool() const;
	//Returns the Item as Furniture. Should only be called if IsFurniture returns true.
	FurnitureP AsFurniture() const;
	//Returns the Item as Clothing. Should only be called if IsClothing returns true.
	ClothingP AsClothing() const;
};

//A Tool is an Item that can be held in hand and may respond to the Action button.
class Tool : public Item
{
public:
	Tool(JSONObject& value, const std::string& filename = "") : Item(value, filename) {}
};

//Furniture is an Item that can be placed down and possibly interacted with.
class Furniture : public Item
{
public:
	enum class Kind
	{
		//Things that go on the floor.
		Houseware,
		//(ACNH internal name for Housewares.)
		Floor = Kind::Houseware,
		//Things that can go on the floor or on top of supporting Housewares.
		Miscellaneous,
		//(ACNH internal name for Miscellaneous)
		Upper = Kind::Miscellaneous,
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
		CeilingRug = Kind::Rug,
		//A bug or fish.
		Creature,
	} Kind{ Kind::Houseware };

	//The item can be placed on either the walls or floor.
	bool canGoOnWallsOrFloor;

	Furniture(JSONObject& value, const std::string& filename = "");
};

class Clothing : public Item
{
public:
	enum class Kind
	{
		Tops,
		Bottoms,
		OnePiece,
		Dress = Kind::OnePiece,
		Hat,
		Cap = Kind::Hat, 
		Helmet = Kind::Hat,
		Accessory,
		Socks,
		Shoes,
		Bag,
		Swimwear,
		MarineSuit = Kind::Swimwear,
	} Kind{ Kind::Tops };

	Clothing(JSONObject& value, const std::string& filename = "");
};

class InventoryItem : public NameableThing
{
private:
	ItemP _wrapped;
	int _variant, _pattern;
	ModelP _model;
	
	std::array<Texture*, 8> Textures;

public:
	InventoryItem(ItemP wrapped, int variant, int pattern);
	InventoryItem(ItemP wrapped, int variant);
	InventoryItem(ItemP wrapped);
	InventoryItem(const std::string& reference);
	std::string FullID();
	std::string FullName();
	//Returns true if the wrapped Item is in fact an Item (a Thing), not a Tool, Furniture, or Clothing.
	bool IsItem() const;
	//Returns true if the wrapped Item is actually a Tool.
	bool IsTool() const;
	//Returns true if the wrapped Item is actually Furniture.
	bool IsFurniture() const;
	//Returns true if the wrapped Item is actually Clothing.
	bool IsClothing() const;
	//Returns the wrapped Item itself, unwrapped.
	ItemP AsItem() const;
	//Returns the wrapped Item as a Tool. Should only be called if IsTool returns true.
	ToolP AsTool() const;
	//Returns the wrapped Item as Furniture. Should only be called if IsFurniture returns true.
	FurnitureP AsFurniture() const;
	//Returns the wrapped Item as Clothing. Should only be called if IsClothing returns true.
	ClothingP AsClothing() const;

	void LoadTextures();
	void AssignTextures(ModelP model);
	void LoadModel();
	ModelP Model();


	//Safe to delete if true. If not, DO NOT DELETE.
	bool Temporary;
};

using InventoryItemP = std::shared_ptr<InventoryItem>;
