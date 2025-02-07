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

	Icon = value["icon"] != nullptr ? value["icon"]->AsString() : "leaf";
	Price = value["price"] != nullptr ? value["price"]->AsInteger() : 0;
	WearLimit = value["breakDamage"] != nullptr ? value["breakDamage"]->AsInteger() : 0;

	if (Type != Type::Clothing)
	{
		StackLimit = value["stack"] != nullptr ? value["stack"]->AsInteger() : 0;

		CanBury = value["canBury"] != nullptr ? value["canBury"]->AsBool() : false;
		CanEat = value["canEat"] != nullptr ? value["canEat"]->AsBool() : false;
		CanPlace = value["canPlace"] != nullptr ? value["canPlace"]->AsBool() : false;
		CanPlant = value["canPlant"] != nullptr ? value["canPlant"]->AsBool() : false;
		CanDrop = value["canDrop"] != nullptr ? value["canDrop"]->AsBool() : false;
		CanGive = value["canGive"] != nullptr ? value["canGive"]->AsBool() : false;
		CanSell = value["canSell"] != nullptr ? value["canSell"]->AsBool() : (Price != 0);
		CanDropOff = value["canDropOff"] != nullptr ? value["canDropOff"]->AsBool() : CanSell;
		CanStore = value["canStore"] != nullptr ? value["canStore"]->AsBool() : false;
		CanTrash = value["canTrash"] != nullptr ? value["canTrash"]->AsBool() : false;
		CanGift = value["canGiveBDay"] != nullptr ? value["canGiveBDay"]->AsBool() : false;
		CanWrap = value["canGiftwrap"] != nullptr ? value["canGiftwrap"]->AsBool() : false;
		CanHave = value["canHave"] != nullptr ? value["canHave"]->AsBool() : true;

		if (Type == Type::Furniture)
		{
			auto kind = value["category"] != nullptr ? value["category"]->AsString() : "[missing value]";
			if (kind == "housewares") FurnKind = FurnKind::Houseware;
			else if (kind == "houseware") FurnKind = FurnKind::Houseware;
			else if (kind == "floor") FurnKind = FurnKind::Houseware;
			else if (kind == "miscellaneous") FurnKind = FurnKind::Miscellaneous;
			else if (kind == "others") FurnKind = FurnKind::Miscellaneous; //TODO: look into this.
			else if (kind == "misc") FurnKind = FurnKind::Miscellaneous;
			else if (kind == "upper") FurnKind = FurnKind::Miscellaneous;
			else if (kind == "wall") FurnKind = FurnKind::Wall;
			else if (kind == "ceiling") FurnKind = FurnKind::Ceiling;
			else if (kind == "roomwall") FurnKind = FurnKind::RoomWall;
			else if (kind == "roomfloor") FurnKind = FurnKind::RoomFloor;
			else if (kind == "rug") FurnKind = FurnKind::Rug;
			else if (kind == "ceilingrug") FurnKind = FurnKind::Rug;
			else if (kind == "creature") FurnKind = FurnKind::Creature;
			else
				throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", kind, ID));

			CanGoOnWallsOrFloor = value["canWallFloor"] != nullptr ? value["canWallFloor"]->AsBool() : false;
		}
	}
	else
	{
		auto kind = value["category"] != nullptr ? value["category"]->AsString() : "[missing value]";
		if (kind == "tops") ClothingKind = ClothingKind::Tops;
		else if (kind == "bottoms") ClothingKind = ClothingKind::Bottoms;
		else if (kind == "onepiece") ClothingKind = ClothingKind::OnePiece;
		else if (kind == "dress") ClothingKind = ClothingKind::OnePiece;
		else if (kind == "hat") ClothingKind = ClothingKind::Hat;
		else if (kind == "cap") ClothingKind = ClothingKind::Cap;
		else if (kind == "helmet") ClothingKind = ClothingKind::Helmet;
		else if (kind == "accessory") ClothingKind = ClothingKind::Accessory;
		else if (kind == "socks") ClothingKind = ClothingKind::Socks;
		else if (kind == "shoes") ClothingKind = ClothingKind::Shoes;
		else if (kind == "bag") ClothingKind = ClothingKind::Bag;
		else if (kind == "swimwear") ClothingKind = ClothingKind::Swimwear;
		else if (kind == "marinesuit") ClothingKind = ClothingKind::Swimwear;
		else
			throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", kind, ID));

		Style = value["style"] != nullptr ? value["style"]->AsString() : "";
		PlayerModel = value["playerModel"] != nullptr ? value["playerModel"]->AsString() : "";
	}
	
	auto vars = value["variants"];
	auto remake = value["remake"];
	if (vars != nullptr)
	{
		for (auto v : vars->AsArray())
			variantNames.push_back(v->AsString());
	}
	else if (remake != nullptr)
	{
		auto bodies = remake->AsObject().at("bodies");
		for (auto v : bodies->AsArray())
			variantNames.push_back(v->AsString());
	}
}

bool Item::IsThing() const
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

void Item::DrawFieldIcon(const glm::vec3& position)
{
	if (!iconModel)
		iconModel = std::make_shared<Model>(fmt::format("field/icons/{}/model.fbx", Icon));
	iconModel->Draw(position);
}

InventoryItem::InventoryItem(ItemP wrapped, int variant, int pattern)
{
	_wrapped = wrapped;
	_variant = variant;
	_pattern = pattern;
	_wear = 0;
	ID = wrapped->ID;
	Hash = wrapped->Hash;
	RefName = wrapped->RefName;
	EnName = wrapped->EnName;
	Path = wrapped->Path;
	Temporary = false;
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
	_wear = 0;

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
		if (_wrapped->variantNames.size() != 0)
		{
			//Accept both variant names and indices. If one fails...
			//This way, definitions can be more clear but saved data
			//doesn't need to have them called by name.
			_variant = _wrapped->FindVariantByName(varNames[0]);
			if (_variant == -1)
				_variant = std::stoi(varNames[0]);
			//TODO: pattern names
			if (varNames.size() > 1)
				_pattern = std::stoi(varNames[1]);
			if (_wrapped->WearLimit != 0)
			{
				_wear = std::stoi(varNames[2]);
			}
		}
		else if (_wrapped->WearLimit != 0)
		{
			_wear = std::stoi(varNames[0]);
		}
	}
	ID = _wrapped->ID;
	Hash = _wrapped->Hash;
	RefName = _wrapped->RefName;
	EnName = _wrapped->EnName;
	Path = _wrapped->Path;
	Temporary = false;
}

std::string InventoryItem::FullID() const
{
	if (_wrapped->variantNames.size() != 0)
	{
		if (_wear > 0)
			return fmt::format("{}/{}/{}/{}", ID, _variant, _pattern, _wear);
		return fmt::format("{}/{}/{}", ID, _variant, _pattern);
	}
	//No variants, but we do have a wear?
	if (_wear > 0)
		return fmt::format("{}/{}", ID, _wear);
	//None of that applies?
	return ID;
}

std::string InventoryItem::FullName()
{
	if (_wear)
	{
		auto wearPct = glm::round((_wrapped->WearLimit * _wear) / 100.0f);
		if (_wrapped->variantNames.size() != 0)
			return fmt::format("{} ({}%, {})", Name(), wearPct, _wrapped->variantNames[_variant]);
		return fmt::format("{} ({}%)", Name(), wearPct);
	}
	if (_wrapped->variantNames.size() != 0)
	{
		return fmt::format("{} ({})", Name(), _wrapped->variantNames[_variant]);
	}
	return Name();
}

ItemP InventoryItem::AsItem() const
{
	return std::static_pointer_cast<Item>(_wrapped);
}

bool InventoryItem::IsThing() const
{
	return _wrapped->IsThing();
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

std::string InventoryItem::Icon() const
{
	return _wrapped->Icon;
}

std::string InventoryItem::Style() const
{
	return _wrapped->Style;
}

std::string InventoryItem::PlayerModel() const
{
	return _wrapped->PlayerModel;
}

int InventoryItem::Variant() const
{
	return _variant;
}

void InventoryItem::Variant(int variant)
{
	_variant = glm::clamp(variant, 0, (int)_wrapped->variantNames.size());
}

bool InventoryItem::WearDown(int howMuch)
{
	if (_wrapped->WearLimit == 0)
		return false;
	_wear += howMuch;
	return _wear >= _wrapped->WearLimit;
}

ItemP InventoryItem::Wrapped() const
{
	return _wrapped;
}
