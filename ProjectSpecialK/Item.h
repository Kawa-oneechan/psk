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

enum ItemType
{
	it_Item         = 0b0000'0000'0000'0001,
	it_Tool         = 0b0000'0000'0000'0011,
	it_Furniture    = 0b0000'0000'0000'0101,
	it_WallPiece    = 0b0000'0001'0000'0101,
	it_CeilingPiece	= 0b0000'0010'0000'0101,
	it_Outfit       = 0b0000'0000'0000'0111,
	it_Tops         = 0b0000'0001'0000'0111,
	it_Bottom       = 0b0000'0010'0000'0111,
	it_Dress        = 0b0000'0011'0000'0111,
	//????          = 0b0000'0011'0000'0111,
	it_Hat          = 0b0000'0100'0000'0111,
	it_Shoes        = 0b0000'0101'0000'0111,
};

class Tool;
class Furniture;
class Outfit;

class Item : public NameableThing
{
public:
	Item(JSONObject& value, const std::string& filename = "");
	int FindVariantByName(const std::string& variantName) const;
	std::vector<std::string> variantNames;
	int Type;
	bool IsItem() const;
	bool IsTool() const;
	bool IsFurniture() const;
	bool IsOutfit() const;
	Item* AsItem() const;
	Tool* AsTool() const;
	Furniture* AsFurniture() const;
	Outfit* AsOutfit() const;
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
