#pragma once

class NameableThing
{
public:
	std::string ID;
	std::string RefName;
	std::string EnName;
	int Type;
	NameableThing(JSONObject& value);
	NameableThing() = default;
	const std::string Name();
};

#define it_Item			0b0000'0000'0000'0001
#define it_Tool			0b0000'0000'0000'0011
#define it_Furniture	0b0000'0000'0000'0101
#define it_WallPiece	0b0000'0001'0000'0101
#define it_CeilingPiece	0b0000'0010'0000'0101
#define it_Outfit		0b0000'0000'0000'0111
#define it_Tops			0b0000'0001'0000'0111
#define it_Bottom		0b0000'0010'0000'0111
#define it_Dress		0b0000'0011'0000'0111
#define it_Hat			0b0000'0100'0000'0111
#define it_Shoes		0b0000'0101'0000'0111

class Item : public NameableThing
{
public:
	Item(JSONObject& value);
	int FindVariantByName(const std::string& variantName) const;
	std::vector<std::string> variantNames;
};

class Tool : public Item
{
public:
	Tool(JSONObject& value) : Item(value) {}
};

class Furniture : public Item
{
public:
	Furniture(JSONObject& value) : Item(value) {}
};

class Outfit : public Item
{
public:
	Outfit(JSONObject& value) : Item(value) {}
};

class InventoryItem : public NameableThing
{
private:
	Item* _wrapped;
	int _variant, _pattern;
public:
	InventoryItem(Item* wrapped, int variant, int pattern);
	InventoryItem(Item* wrapped, int variant);
	InventoryItem(Item* wrapped);
	InventoryItem(const std::string& reference);
	const std::string FullID();
	const std::string FullName();
	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsOutfit() const;
	Item* AsItem() const;
	Tool* AsTool() const;
	Furniture* AsFurniture() const;
	Outfit* AsOutfit() const;
	//Safe to delete if true. If not, DO NOT DELETE.
	bool Temporary;
};
