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

void InventoryItem::LoadTextures()
{
	if (Textures[0] == nullptr)
	{
		Textures[0] = new Texture(fmt::format("{}/albedo{}.png", Path, _variant));
		Textures[1] = new Texture(fmt::format("{}/normal.png", Path));
		Textures[2] = new Texture(fmt::format("{}/mix.png", Path));
		Textures[3] = new Texture(fmt::format("{}/opacity.png", Path));
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
