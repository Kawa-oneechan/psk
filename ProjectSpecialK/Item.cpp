#include "engine/TextUtils.h"
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

Item::Item(jsonObject& value, const std::string& filename) : NameableThing(value, filename)
{
	auto type = value["type"].is_string() ? value["type"].as_string() : "[missing value]";
	if (type == "thing") Type = Type::Thing;
	else if (type == "tool") Type = Type::Tool;
	else if (type == "furniture") Type = Type::Furniture;
	else if (type == "clothes") Type = Type::Clothing;
	else if (type == "clothing") Type = Type::Clothing;
	else if (type == "outfit") Type = Type::Clothing;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", type, ID));

	Icon = value["icon"].is_string() ? value["icon"].as_string() : "leaf";
	Price = value["price"].is_integer() ? value["price"].as_integer() : 0;
	WearLimit = value["breakDamage"].is_integer() ? value["breakDamage"].as_integer() : 0;

	if (Type != Type::Clothing)
	{
		StackLimit = value["stack"].is_integer() ? value["stack"].as_integer() : 0;

		CanBury = value["canBury"].is_boolean() ? value["canBury"].as_boolean() : false;
		CanEat = value["canEat"].is_boolean() ? value["canEat"].as_boolean() : false;
		CanPlace = value["canPlace"].is_boolean() ? value["canPlace"].as_boolean() : false;
		CanPlant = value["canPlant"].is_boolean() ? value["canPlant"].as_boolean() : false;
		CanDrop = value["canDrop"].is_boolean() ? value["canDrop"].as_boolean() : false;
		CanGive = value["canGive"].is_boolean() ? value["canGive"].as_boolean() : false;
		CanSell = value["canSell"].is_boolean() ? value["canSell"].as_boolean() : (Price != 0);
		CanDropOff = value["canDropOff"].is_boolean() ? value["canDropOff"].as_boolean() : CanSell;
		CanStore = value["canStore"].is_boolean() ? value["canStore"].as_boolean() : false;
		CanTrash = value["canTrash"].is_boolean() ? value["canTrash"].as_boolean() : false;
		CanGift = value["canGiveBDay"].is_boolean() ? value["canGiveBDay"].as_boolean() : false;
		CanWrap = value["canGiftwrap"].is_boolean() ? value["canGiftwrap"].as_boolean() : false;
		CanHave = value["canHave"].is_boolean() ? value["canHave"].as_boolean() : true;

		if (Type == Type::Furniture)
		{
			auto kind = value["category"].is_string() ? value["category"].as_string() : "[missing value]";
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

			CanGoOnWallsOrFloor = value["canWallFloor"].is_boolean() ? value["canWallFloor"].as_boolean() : false;
		}
	}
	else
	{
		auto kind = value["category"].is_string() ? value["category"].as_string() : "[missing value]";
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

		Style = value["style"].is_string() ? value["style"].as_string() : "";
		PlayerModel = value["playerModel"].is_string() ? value["playerModel"].as_string() : "";
	}
	
	auto vars = value["variants"];
	auto remake = value["remake"];
	if (vars.is_array())
	{
		for (auto v : vars.as_array())
			variantNames.push_back(v.as_string());
	}
	else if (remake.is_array())
	{
		auto bodies = remake.as_object().at("bodies");
		for (auto v : bodies.as_array())
			variantNames.push_back(v.as_string());
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

void Item::DrawFieldModel(const glm::vec3& position, float facing)
{
	if (!fieldModel)
		fieldModel = std::make_shared<Model>(fmt::format("{}/model.fbx", Path));
	fieldModel->Draw(position, facing);
}

InventoryItem::InventoryItem(ItemP wrapped, int data)
{
	_wrapped = wrapped;
	_data = data;
	_wear = 0;
	_packaging = 0;
	ID = wrapped->ID;
	Hash = wrapped->Hash;
	RefName = wrapped->RefName;
	EnName = wrapped->EnName;
	Path = wrapped->Path;
	Temporary = false;
}

InventoryItem::InventoryItem(ItemP wrapped, int variant, int pattern) : InventoryItem(wrapped, (pattern * 32) + variant)
{}

InventoryItem::InventoryItem(ItemP wrapped) : InventoryItem(wrapped, 0)
{}

InventoryItem::InventoryItem(const std::string& reference)
{
	auto cleanName = reference;
	auto slash = cleanName.find_first_of('/');

	_data = 0;
	_wear = 0;
	_packaging = 0;

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
		/* Acceptable formats:
		* id
		* id/variantName
		* id/variantName/patternName
		* id/variantName/patternIndex
		* id/variantName/patternName/packaging
		* id/variantName/patternIndex/packaging
		* id/count
		* id/count/wear
		* id/count/wear/packaging
		*/

		if (_wrapped->variantNames.size() != 0)
		{
			auto variant = _wrapped->FindVariantByName(varNames[0]);
			if (variant == -1)
			{
				//Not a variant. Use Count to set both variant and pattern.
				_data = std::stoi(varNames[0]);
				if (varNames.size() > 1 && _wrapped->WearLimit != 0)
					_wear = std::stoi(varNames[1]);
				if (varNames.size() > 2)
					_packaging = std::stoi(varNames[2]);
			}
			else
			{
				//It was a variant name.
				_data = variant;
				//auto pattern = 0;
				if (varNames.size() > 1)
				{
					//TODO: pattern names
					//pattern = _wrapped->FindPatternByName(varNames[1]);
					//if (pattern == -1)
					//pattern = std::stoi(varNames[1]);
					Pattern(std::stoi(varNames[1]));
					if (varNames.size() > 2)
						_packaging = std::stoi(varNames[2]);
				}
			}
		}
		else
		{
			_data = std::stoi(varNames[0]);
			if (varNames.size() > 1 && _wrapped->WearLimit != 0)
				_wear = std::stoi(varNames[1]);
			if (varNames.size() > 2)
				_packaging = std::stoi(varNames[2]);
		}
	}

	ID = _wrapped->ID;
	Hash = _wrapped->Hash;
	RefName = _wrapped->RefName;
	EnName = _wrapped->EnName;
	Path = _wrapped->Path;
	Temporary = false;
}

InventoryItem::InventoryItem(hash hash)
{
	_wrapped = Database::Find<::Item>(hash, items);
	if (!_wrapped)
		_wrapped = Database::Find<::Item>("psk:thingfallback", items);
	ID = _wrapped->ID;
	Hash = _wrapped->Hash;
	RefName = _wrapped->RefName;
	EnName = _wrapped->EnName;
	Path = _wrapped->Path;
	Temporary = false;
}

std::string InventoryItem::FullID() const
{
	//We don't need variant/pattern splits for loading and saving bro.
	if (_wear > 0)
	{
		if (_packaging != 0)
			return fmt::format("{}/{}/{}/{}", ID, _data, _wear, _packaging);
		return fmt::format("{}/{}/{}", ID, _data, _wear);
	}
	if (_data != 0)
	{
		if (_packaging != 0)
			return fmt::format("{}/{}/0/{}", ID, _data, _packaging);
		return fmt::format("{}/{}", ID, _data);
	}
	if (_packaging != 0)
	{
		return fmt::format("{}/0/0/{}", ID, _packaging);
	}
	return ID;
}

std::string InventoryItem::FullName()
{
	if ((_packaging & 0x7F) != 0)
	{
		//auto packageType = _packaging & 0x7F;
		if ((_packaging & 0x80) == 0x80)
			return "present";
		//If the bit is clear, we want to show the true name.
		//TODO: use Text::Get
	}

	if (_wear)
	{
		auto wearPct = glm::round((_wrapped->WearLimit * _wear) / 100.0f);
		if (_wrapped->variantNames.size() != 0)
			return fmt::format("{} ({}%, {})", Name(), wearPct, _wrapped->variantNames[_data % 32]);
		return fmt::format("{} ({}%)", Name(), wearPct);
	}
	if (_wrapped->variantNames.size() != 0)
	{
		return fmt::format("{} ({})", Name(), _wrapped->variantNames[_data % 32]);
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
	if ((_packaging & 0x80) != 0)
	{
		//auto packageType = _packaging & 0x7F;
		return "present";
	}
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

int InventoryItem::Data() const
{
	return _data;
}

void InventoryItem::Data(int data)
{
	_data = data;
}

int InventoryItem::Variant() const
{
	return _data % 32;
}

void InventoryItem::Variant(int variant)
{
	_data = (Pattern() * 32) + glm::clamp(variant, 0, (int)_wrapped->variantNames.size());
}

int InventoryItem::Pattern() const
{
	return _data / 32;
}

void InventoryItem::Pattern(int pattern)
{
	_data = (pattern * 32) + Variant();
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
