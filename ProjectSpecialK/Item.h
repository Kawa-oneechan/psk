#pragma once

class NameableThing
{
public:
	std::string ID;
	std::string RefName;
	std::string EnName;
	std::string Path; //To locate specific stuff like models, textures, sounds...
	unsigned int Hash;
	NameableThing(JSONObject& value, const std::string& filename = "");
	NameableThing() = default;
	const std::string Name();

	inline bool operator== (const NameableThing& r) { return this->ID == r.ID; }
	inline bool operator!= (const NameableThing& r) { return this->ID != r.ID; }
	inline bool operator== (const std::string& r) { return this->ID == r; }
	inline bool operator!= (const std::string& r) { return this->ID != r; }
	inline bool operator== (unsigned int r) { return this->Hash == r; }
	inline bool operator!= (unsigned int r) { return this->Hash != r; }
};

class Item;
class Tool;
class Furniture;
class Outfit;

using ItemP = std::shared_ptr<Item>;
using ToolP = std::shared_ptr<Tool>;
using FurnitureP = std::shared_ptr<Furniture>;
using OutfitP = std::shared_ptr<Outfit>;

class Item : public NameableThing
{
public:
	Item(JSONObject& value, const std::string& filename = "");
	int FindVariantByName(const std::string& variantName) const;
	std::vector<std::string> variantNames;

	enum Type
	{
		Generic = 0b0000'0000'0000'0001,
		Tool = 0b0000'0000'0000'0011,
		Furniture = 0b0000'0000'0000'0101,
		WallPiece = 0b0000'0001'0000'0101,
		CeilingPiece = 0b0000'0010'0000'0101,
		Outfit = 0b0000'0000'0000'0111,
		Tops = 0b0000'0001'0000'0111,
		Bottom = 0b0000'0010'0000'0111,
		Dress = 0b0000'0011'0000'0111,
		//???? = 0b0000'0011'0000'0111,
		Hat = 0b0000'0100'0000'0111,
		Shoes = 0b0000'0101'0000'0111,
	} Type{ Type::Generic };
	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsOutfit() const;
	ItemP AsItem() const;
	ToolP AsTool() const;
	FurnitureP AsFurniture() const;
	OutfitP AsOutfit() const;
};

class Tool : public Item
{
public:
	Tool(JSONObject& value, const std::string& filename = "") : Item(value, filename) {}
};

class Furniture : public Item
{
public:
	Furniture(JSONObject& value, const std::string& filename = "") : Item(value, filename) {}
};

class Outfit : public Item
{
public:
	Outfit(JSONObject& value, const std::string& filename = "") : Item(value, filename) {}
};

class InventoryItem : public NameableThing
{
private:
	ItemP _wrapped;
	int _variant, _pattern;
public:
	InventoryItem(ItemP wrapped, int variant, int pattern);
	InventoryItem(ItemP wrapped, int variant);
	InventoryItem(ItemP wrapped);
	InventoryItem(const std::string& reference);
	const std::string FullID();
	const std::string FullName();
	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsOutfit() const;
	ItemP AsItem() const;
	ToolP AsTool() const;
	FurnitureP AsFurniture() const;
	OutfitP AsOutfit() const;
	//Safe to delete if true. If not, DO NOT DELETE.
	bool Temporary;
};

using InventoryItemP = std::shared_ptr<InventoryItem>;
