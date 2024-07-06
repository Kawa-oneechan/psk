#include "SpecialK.h"

extern int articlePlease;

NameableThing::NameableThing(JSONObject& value, const std::string& filename)
{
	ID = value["id"]->AsString();
	Hash = GetCRC(ID);
	
	auto ref = fmt::format("name:{}", ID);
	StringToLower(ref);
	StripSpaces(ref);
	RefName = ref;

	if (!filename.empty())
		Path = filename.substr(0, filename.find_last_of('/'));
	else
		Path.clear();
	
	auto val = value["name"];
	if (val->IsArray())
	{
		//We may be trying to load a Species, which has masculine and feminine names.
		//Don't even bother setting EnName, as Species hides it with EnName[].
		return;
	}
	if (val->IsString() && val->AsString()[0] == '#')
		RefName = ref = val->AsString().substr(1);
	else
		TextAdd(ref, *val);

	EnName = StripMSBT(TextGet(ref, Language::EUen));
}

const std::string NameableThing::Name()
{
	auto text = TextGet(RefName);
	if (text.substr(0, 6) == "<info:")
	{
		auto msbtEnd = text.find_first_of('>', 7);
		auto rest = text.substr(msbtEnd + 1);
		if (articlePlease)
		{
			auto msbtWhole = text.substr(7, msbtEnd - 7);
			auto msbt = Split(msbtWhole, ':');
			auto art = articlePlease - 1;
			articlePlease = 0;
			return msbt[art] + rest;
		}
		else
			return rest;
	}
	return text;
}

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
	auto type = value["type"] != nullptr ? value["type"]->AsString() : "";
	if (type.empty());
	else if (type == "tool") Type = Type::Tool;
	else if (type == "furniture") Type = Type::Furniture;
	else if (type == "tops") Type = Type::Tops;
	else if (type == "bottom") Type = Type::Bottom;
	else if (type == "bottoms") Type = Type::Bottom;
	else if (type == "dress") Type = Type::Dress;
	else if (type == "dressup") Type = Type::Dress;
	else if (type == "onepiece") Type = Type::Dress;
	else if (type == "hat") Type = Type::Hat;
	else if (type == "cap") Type = Type::Hat;
	else if (type == "shoes") Type = Type::Shoes;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", type, ID));

	auto vars = value["variants"];
	if (vars != nullptr)
	{
		for (auto v : vars->AsArray())
			variantNames.push_back(v->AsString());
	}
}


bool Item::IsItem() const
{
	return Type != 0;
}

bool Item::IsTool() const
{
	return (Type & Type::Tool) == Type::Tool;
}

bool Item::IsFurniture() const
{
	return (Type & Type::Furniture) == Type::Furniture;
}

bool Item::IsOutfit() const
{
	return (Type & Type::Outfit) == Type::Outfit;
}

ItemP Item::AsItem() const
{
	//TODO: CHECK THIS
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

OutfitP Item::AsOutfit() const
{
	return std::make_shared<::Outfit>(*(::Outfit*)this);
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
	Temporary = false;
}

const std::string InventoryItem::FullID()
{
	if (_wrapped->variantNames.size() != 0)
	{
		//TODO: patterns
		return fmt::format("{}/{}", ID, _wrapped->variantNames[_variant]);
	}
	return ID;
}

const std::string InventoryItem::FullName()
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

bool InventoryItem::IsOutfit() const
{
	return _wrapped->IsOutfit();
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

OutfitP InventoryItem::AsOutfit() const
{
	return std::static_pointer_cast<Outfit>(_wrapped);
}
