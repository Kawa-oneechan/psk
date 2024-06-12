#include "SpecialK.h"

extern "C" uint32_t crc_32(const unsigned char *input_str, size_t num_bytes);

extern int articlePlease;

NameableThing::NameableThing(JSONObject& value)
{
	ID = value["id"]->AsString();
	Hash = crc_32((unsigned char*)ID.c_str(), ID.length());
	auto ref = fmt::format("name:{}", ID);
	StringToLower(ref);
	StripSpaces(ref);
	RefName = ref;

	auto val = value["name"];
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

Item::Item(JSONObject& value) : NameableThing(value)
{
	//auto vars = value.find("variants");
	//if (vars != value.end())

	Type = it_Item;

	auto type = value["type"] != nullptr ? value["type"]->AsString() : "";
	if (type.empty());
	else if (type == "tool") Type = it_Tool;
	else if (type == "furniture") Type = it_Furniture;
	else if (type == "tops") Type = it_Tops;
	else if (type == "bottom") Type = it_Bottom;
	else if (type == "bottoms") Type = it_Bottom;
	else if (type == "dress") Type = it_Dress;
	else if (type == "dressup") Type = it_Dress;
	else if (type == "onepiece") Type = it_Dress;
	else if (type == "hat") Type = it_Hat;
	else if (type == "cap") Type = it_Hat;
	else if (type == "shoes") Type = it_Shoes;
	else
		throw std::runtime_error(fmt::format("Don't know what to do with type \"{}\" while loading {}.", type, ID));
	//fmt::print("Don't know what to do with type \"{}\".\n", type);

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
	return (Type & it_Tool) == it_Tool;
}

bool Item::IsFurniture() const
{
	return (Type & it_Furniture) == it_Furniture;
}

bool Item::IsOutfit() const
{
	return (Type & it_Outfit) == it_Outfit;
}

Item* Item::AsItem() const
{
	return (Item*)this;
}

Tool* Item::AsTool() const
{
	return (Tool*)this;
}

Furniture* Item::AsFurniture() const
{
	return (Furniture*)this;
}

Outfit* Item::AsOutfit() const
{
	return (Outfit*)this;
}

InventoryItem::InventoryItem(Item * wrapped, int variant, int pattern)
{
	_wrapped = wrapped;
	_variant = variant;
	_pattern = pattern;
	ID = wrapped->ID;
	RefName = wrapped->RefName;
	EnName = wrapped->EnName;
	Temporary = false;
}

InventoryItem::InventoryItem(Item * wrapped, int variant) : InventoryItem(wrapped, variant, 0)
{}

InventoryItem::InventoryItem(Item * wrapped) : InventoryItem(wrapped, 0, 0)
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
	_wrapped = Database::Find<::Item>(cleanName, &items);
	if (_wrapped == nullptr)
	{
		_wrapped = Database::Find<::Item>("psk:toolfallback", &items);
	}
	else if (slash != std::string::npos)
	{
		auto varNames = Split(reference.substr(slash + 1), '/');
		_variant = _wrapped->FindVariantByName(varNames[0]);
		//TODO: pattern
		_pattern = 0;
	}
	ID = _wrapped->ID;
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

Item* InventoryItem::AsItem() const
{
	return (Item*)_wrapped;
}

Tool* InventoryItem::AsTool() const
{
	return (Tool*)_wrapped;
}

Furniture* InventoryItem::AsFurniture() const
{
	return (Furniture*)_wrapped;
}

Outfit* InventoryItem::AsOutfit() const
{
	return (Outfit*)_wrapped;
}
