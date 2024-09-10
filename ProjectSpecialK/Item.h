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

	int price{ 0 };
	int stackLimit{ 0 };

	bool canBury{ false };
	bool canEat{ false };
	bool canPlace{ false };
	bool canPlant{ false };
	bool canDrop{ false };
	bool canGive{ false };
	bool canSell{ false };
	bool canDropOff{ false };
	bool canStore{ false };
	bool canTrash{ false };
	bool canGift{ false };
	bool canWrap{ false };

	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsClothing() const;
	ItemP AsItem() const;
	ToolP AsTool() const;
	FurnitureP AsFurniture() const;
	ClothingP AsClothing() const;
};

class Tool : public Item
{
public:
	Tool(JSONObject& value, const std::string& filename = "") : Item(value, filename) {}
};

class Furniture : public Item
{
public:
	enum class Kind
	{
		Housewares,
		Floor = Kind::Housewares,
		Miscellanous,
		Upper = Kind::Miscellanous,
		Wall,
		Ceiling,
		RoomWall,
		RoomFloor,
		Rug,
		CeilingRug = Kind::Rug,
		Creature,
	} Kind{ Kind::Housewares };

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
	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsClothing() const;
	ItemP AsItem() const;
	ToolP AsTool() const;
	FurnitureP AsFurniture() const;
	ClothingP AsClothing() const;

	void LoadTextures();
	void AssignTextures(ModelP model);
	void LoadModel();
	ModelP Model();


	//Safe to delete if true. If not, DO NOT DELETE.
	bool Temporary;
};

using InventoryItemP = std::shared_ptr<InventoryItem>;
