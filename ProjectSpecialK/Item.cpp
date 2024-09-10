#include "SpecialK.h"

int Item::FindVariantByName(const std::string& variantName) const
{
	for (int i = 0; i < variantNames.size(); i++)
	{
		if (variantNames[i] == variantName)
			return i;
	}
	return -1;
}

Item::Item(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	auto type = value["type"] != nullptr ? value["type"]->AsString() : "[missing value]";
	if (type == "thing") Type = Type::Thing;
	else if (type == "tool") Type = Type::Tool;
	else if (type == "furniture") Type = Type::Furniture;
	else if (type == "clothes") Type = Type::Clothing;
	else if (type == "clothing") Type = Type::Clothing;
	else if (type == "outfit") Type = Type::Clothing;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", type, ID));

	price = value["price"] != nullptr ? (int)value["price"]->AsNumber() : 0;

	if (Type != Type::Clothing)
	{
		stackLimit = value["stack"] != nullptr ? (int)value["stack"]->AsNumber() : 0;

		canBury = value["canBury"] != nullptr ? value["canBury"]->AsBool() : false;
		canEat = value["canEat"] != nullptr ? value["canEat"]->AsBool() : false;
		canPlace = value["canPlace"] != nullptr ? value["canPlace"]->AsBool() : false;
		canPlant = value["canPlant"] != nullptr ? value["canPlant"]->AsBool() : false;
		canDrop = value["canDrop"] != nullptr ? value["canDrop"]->AsBool() : false;
		canGive = value["canGive"] != nullptr ? value["canGive"]->AsBool() : false;
		canSell = value["canSell"] != nullptr ? value["canSell"]->AsBool() : (price != 0);
		canDropOff = value["canDropOff"] != nullptr ? value["canDropOff"]->AsBool() : canSell;
		canStore = value["canStore"] != nullptr ? value["canStore"]->AsBool() : false;
		canTrash = value["canTrash"] != nullptr ? value["canTrash"]->AsBool() : false;
		canGift = value["canGiveBDay"] != nullptr ? value["canGiveBDay"]->AsBool() : false;
		canWrap = value["canGiftwrap"] != nullptr ? value["canGiftwrap"]->AsBool() : false;
	}

	auto vars = value["variants"];
	if (vars != nullptr)
	{
		for (auto v : vars->AsArray())
			variantNames.push_back(v->AsString());
	}
}

bool Item::IsItem() const
{
	return Type == Type::Thing;
}

bool Item::IsTool() const
{
	return Type == Type::Tool;
}

bool Item::IsFurniture() const
{
	return Type == Type::Furniture;
}

bool Item::IsClothing() const
{
	return Type == Type::Clothing;
}

ItemP Item::AsItem() const
{
	return std::make_shared<Item>(*this);
}

ToolP Item::AsTool() const
{
	return std::make_shared<::Tool>(*(::Tool*)this);
}

FurnitureP Item::AsFurniture() const
{
	return std::make_shared<::Furniture>(*(::Furniture*)this);
}

ClothingP Item::AsClothing() const
{
	return std::make_shared<::Clothing>(*(::Clothing*)this);
}

Furniture::Furniture(JSONObject& value, const std::string& filename) : Item(value, filename)
{
	auto kind = value["category"] != nullptr ? value["category"]->AsString() : "[missing value]";
	if (kind == "housewares") Kind = Kind::Houseware;
	else if (kind == "houseware") Kind = Kind::Houseware;
	else if (kind == "floor") Kind = Kind::Houseware;
	else if (kind == "miscellaneous") Kind = Kind::Miscellaneous;
	else if (kind == "others") Kind = Kind::Miscellaneous; //TODO: look into this.
	else if (kind == "misc") Kind = Kind::Miscellaneous;
	else if (kind == "upper") Kind = Kind::Miscellaneous;
	else if (kind == "wall") Kind = Kind::Wall;
	else if (kind == "ceiling") Kind = Kind::Ceiling;
	else if (kind == "roomwall") Kind = Kind::RoomWall;
	else if (kind == "roomfloor") Kind = Kind::RoomFloor;
	else if (kind == "rug") Kind = Kind::Rug;
	else if (kind == "ceilingrug") Kind = Kind::Rug;
	else if (kind == "creature") Kind = Kind::Creature;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", kind, ID));

	canGoOnWallsOrFloor = value["canWallFloor"] != nullptr ? value["canWallFloor"]->AsBool() : false;
}

Clothing::Clothing(JSONObject& value, const std::string& filename) : Item(value, filename)
{
	auto kind = value["category"] != nullptr ? value["category"]->AsString() : "[missing value]";
	if (kind == "tops") Kind = Kind::Tops;
	else if (kind == "bottoms") Kind = Kind::Bottoms;
	else if (kind == "onepiece") Kind = Kind::OnePiece;
	else if (kind == "dress") Kind = Kind::OnePiece;
	else if (kind == "hat") Kind = Kind::Hat;
	else if (kind == "cap") Kind = Kind::Cap;
	else if (kind == "helmet") Kind = Kind::Helmet;
	else if (kind == "accessory") Kind = Kind::Accessory;
	else if (kind == "socks") Kind = Kind::Socks;
	else if (kind == "shoes") Kind = Kind::Shoes;
	else if (kind == "bag") Kind = Kind::Bag;
	else if (kind == "swimwear") Kind = Kind::Swimwear;
	else if (kind == "marinesuit") Kind = Kind::Swimwear;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", kind, ID));
}

InventoryItem::InventoryItem(ItemP wrapped, int variant, int pattern)
{
	_wrapped = wrapped;
	_variant = variant;
	_pattern = pattern;
	ID = wrapped->ID;
	Hash = wrapped->Hash;
	RefName = wrapped->RefName;
	EnName = wrapped->EnName;
	Path = wrapped->Path;
	Temporary = false;
	Textures.fill(nullptr);
}

InventoryItem::InventoryItem(ItemP wrapped, int variant) : InventoryItem(wrapped, variant, 0)
{}

InventoryItem::InventoryItem(ItemP wrapped) : InventoryItem(wrapped, 0, 0)
{}

InventoryItem::InventoryItem(const std::string& reference)
{
	auto cleanName = reference;
	auto slash = cleanName.find_first_of('/');

	_variant = 0;
	_pattern = 0;

	if (slash != std::string::npos)
	{
		cleanName = reference.substr(0, slash);
	}
	_wrapped = Database::Find<::Item>(cleanName, items);
	if (!_wrapped)
	{
		_wrapped = Database::Find<::Item>("psk:toolfallback", items);
	}
	else if (slash != std::string::npos)
	{
		auto k = reference.substr(slash + 1);
		auto varNames = Split(k, '/');
		_variant = _wrapped->FindVariantByName(varNames[0]);
		//TODO: pattern
		_pattern = 0;
	}
	ID = _wrapped->ID;
	Hash = _wrapped->Hash;
	RefName = _wrapped->RefName;
	EnName = _wrapped->EnName;
	Path = _wrapped->Path;
	Textures.fill(nullptr);
	Temporary = false;
}

std::string InventoryItem::FullID()
{
	if (_wrapped->variantNames.size() != 0)
	{
		//TODO: patterns
		return fmt::format("{}/{}", ID, _wrapped->variantNames[_variant]);
	}
	return ID;
}

std::string InventoryItem::FullName()
{
	if (_wrapped->variantNames.size() != 0)
	{
		//TODO: patterns
		return fmt::format("{} ({})", Name(), _wrapped->variantNames[_variant]);
	}
	return Name();
}

bool InventoryItem::IsItem() const
{
	return _wrapped->IsItem();
}

bool InventoryItem::IsTool() const
{
	return _wrapped->IsTool();
}

bool InventoryItem::IsFurniture() const
{
	return _wrapped->IsFurniture();
}

bool InventoryItem::IsClothing() const
{
	return _wrapped->IsClothing();
}

ItemP InventoryItem::AsItem() const
{
	return std::static_pointer_cast<Item>(_wrapped);
}

ToolP InventoryItem::AsTool() const
{
	return std::static_pointer_cast<Tool>(_wrapped);
}

FurnitureP InventoryItem::AsFurniture() const
{
	return std::static_pointer_cast<Furniture>(_wrapped);
}

ClothingP InventoryItem::AsClothing() const
{
	return std::static_pointer_cast<Clothing>(_wrapped);
}

void InventoryItem::LoadTextures()
{
	if (Textures[0] == nullptr)
	{
		/*
		Texture order:
				alb	nml	mix	opc
		body	0	1	2	3
		...
		*/
		Textures[0] = new TextureArray(fmt::format("{}/albedo*.png", Path));
		Textures[1] = new TextureArray(fmt::format("{}/normal.png", Path));
		Textures[2] = new TextureArray(fmt::format("{}/mix.png", Path));
		Textures[3] = new TextureArray(fmt::format("{}/opacity.png", Path));
	}
}

void InventoryItem::AssignTextures(ModelP model)
{
	model->Textures[0] = Textures[0];
	model->Textures[1] = Textures[1];
	model->Textures[2] = Textures[2];
	model->Textures[3] = Textures[3];
}

void InventoryItem::LoadModel()
{
	if (!_model)
		_model = std::make_shared<::Model>(fmt::format("{}/model.fbx", Path));
	LoadTextures();
}

ModelP InventoryItem::Model()
{
	if (!_model)
		LoadModel();
	return _model;
}
