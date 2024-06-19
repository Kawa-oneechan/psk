#include "SpecialK.h"

Villager::Villager(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
//	ID = value["id"]->AsString();

	_model = nullptr;
	_species = nullptr;
	gender = Gender::BEnby;
	_birthday[0] = 0;
	_birthday[1] = 0;
	memset(_flags, 0, sizeof(_flags));
	memset(_memories, 0, sizeof(_memories));
	personality = nullptr;
	personalitySubtype = 0;
	hobby = nullptr;
	HeldTool = nullptr;
	Hat = nullptr;
	Glasses = nullptr;
	Mask = nullptr;
	Outfit = nullptr;

	_customModel = value["customModel"] != nullptr && value["customModel"]->IsBool() ? value["customModel"]->AsBool() : false;
	_isSpecial = value["isSpecial"] != nullptr && value["isSpecial"]->IsBool() ? value["isSpecial"]->AsBool() : false;
	
	if (_isSpecial)
	{
		//_species = nullptr;
		RefSpecies = "<<special>>";
	}
	else
	{
		auto sp = value["species"];
		_species = Database::Find<::Species>(sp, &species);
		if (_species == nullptr)
			throw std::runtime_error(fmt::format("Unknown species {} while loading {}.", sp->Stringify(), ID));
		RefSpecies = fmt::format("species:{}", _species->ID);
		StringToLower(RefSpecies);
	}
	_model = nullptr;

	//Normally, special villagers have no catchphrase but we'll allow it as an option.
	RefCatchphrase = fmt::format("catchphrase:{}", ID);
	auto val = value["catchphrase"];
	if (val == nullptr || (val->IsString() && val->AsString().empty()))
		val = JSON::Parse("\"dummy\"");
	if (val->IsString() && val->AsString()[0] == '#')
		RefCatchphrase = val->AsString().substr(1);
	else
		TextAdd(RefCatchphrase, *val);

	auto birthday = value["birthday"]->AsArray();
	_birthday[0] = (int)birthday[0]->AsNumber();
	_birthday[1] = (int)birthday[1]->AsNumber();
	if (_birthday[0] < 1 || _birthday[0] > 31 || _birthday[1] < 1 || _birthday[1] > 12)
	{
		conprint(1, "Villager {} has an out of band birthday: day {}, month {}.", ID, _birthday[0], _birthday[1]);
		//Reset to January 1 so nothing will badly break later on.
		_birthday[0] = _birthday[1] = 1;
	}

	auto _gender = value["gender"];
	if (_gender != nullptr)
	{
		if (_gender->AsString() == "mas")
			this->gender = Gender::Boy;
		else if (_gender->AsString() == "fem")
			this->gender = Gender::Girl;
		else if (_gender->AsString() == "mnb")
			this->gender = Gender::BEnby;
		else if (_gender->AsString() == "fnb")
			this->gender = Gender::GEnby;
		else
			throw std::runtime_error(fmt::format("Unknown gender {} while loading {}.", _gender->Stringify(), ID));
	}

	auto nametag = value["nameTag"]->AsArray();
	for (int i = 0; i < 2; i++)
	{
		auto hex = nametag[i]->AsString();
		int r, g, b, a;
		if (hex.empty() || hex[0] != '#')
			throw std::runtime_error(fmt::format("Not a color value {} while loading {}.", hex, ID));
		if (hex.length() == 7)
		{
			a = 0xFF;
			r = std::stoi(hex.substr(1, 2), nullptr, 16);
			g = std::stoi(hex.substr(3, 2), nullptr, 16);
			b = std::stoi(hex.substr(5, 2), nullptr, 16);
		}
		else if (hex.length() == 9)
		{
			a = std::stoi(hex.substr(1, 2), nullptr, 16);
			r = std::stoi(hex.substr(3, 2), nullptr, 16);
			g = std::stoi(hex.substr(5, 2), nullptr, 16);
			b = std::stoi(hex.substr(7, 2), nullptr, 16);
		}
		else
			throw std::runtime_error(fmt::format("Not a well-formed color value {} while loading {}.", hex, ID));
		NameTag[i] = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}

	if (_isSpecial)
		personality = nullptr;
	else
	{
		personality = Database::Find<::Personality>(value["personality"], &personalities);
		if (personality == nullptr)
			throw std::runtime_error(fmt::format("Unknown personality {} while loading {}.", value["personality"]->Stringify(), ID));
	}
	personalitySubtype = _isSpecial ? 0 : (int)value["personalitySubtype"]->AsNumber();
	//hobby = value["hobby"];
	{
		hobby = Database::Find<::Hobby>(value["hobby"], &hobbies);
		if (hobby == nullptr)
		{
			conprint(1, "Unknown hobby {} while loading {}.", value["hobby"]->Stringify(), ID);
			hobby = Database::Find<::Hobby>("fallback", &hobbies);
		}
	}

	if (value["umbrella"] != nullptr) umbrellaID = value["umbrella"]->AsString();
	if (value["photo"] != nullptr) photoID = value["photo"]->AsString();
	if (value["poster"] != nullptr) portraitID = value["poster"]->AsString();
	/*
	umbrella = new InventoryItem(value["umbrella"]->AsString());
	photo = new InventoryItem(value["photo"]->AsString());
	portrait = new InventoryItem(value["poster"]->AsString());
	*/

	//Everything after this point is only for regular villagers.
	if (_isSpecial)
		return;

	//TODO: variations. Right now, those are stripped out while looking up the items.
	auto clothing = value["clothing"]->AsObject();
	defaultOutfitID = clothing["default"]->AsString();
	rainCoatID = (clothing["rain"]->AsArray())[0]->AsString();
	rainHatID = (clothing["rain"]->AsArray())[1]->AsString();
	/*
	defaultOutfit = new InventoryItem(clothing["default"]->AsString()); //ResolveItem(clothing["default"]->AsString(), "topsfallback");
	rainCoat = new InventoryItem((clothing["rain"]->AsArray())[0]->AsString()); //ResolveItem((clothing["rain"]->AsArray())[0]->AsString(), "topsfallback");
	rainHat = new InventoryItem((clothing["rain"]->AsArray())[1]->AsString()); //ResolveItem((clothing["rain"]->AsArray())[1]->AsString(), "hatfallback");
	*/
}

const std::string Villager::Name()
{
	return TextGet(RefName);
}

const std::string Villager::Species()
{
	if (gender == Gender::Girl || gender == Gender::GEnby)
		return TextGet(RefSpecies + ":f");
	else
		return TextGet(RefSpecies + ":m");
}

void Villager::LoadModel()
{
	//If _customModel is set, load our own. If not, look up the model reference for this species.
	//Either way, I'm gonna need some way to reuse and refcount already-loaded models...
}

const Model* Villager::Model()
{
	if (_model == nullptr)
		LoadModel();
	return _model;
}

const std::string Villager::Birthday()
{
	//return TextGet("month" + std::to_string(_birthday[0])) + " " + std::to_string(_birthday[1]);
	return TextDateMD(_birthday[1], _birthday[0]);
}

const std::string Villager::Catchphrase()
{
	if (_customCatchphrase.length())
		return _customCatchphrase;
	return TextGet(RefCatchphrase);
}

std::string Villager::Catchphrase(std::string& newPhrase)
{
	const auto oldPhrase = std::string(_customCatchphrase);
	_customCatchphrase = newPhrase;
	return oldPhrase;
}

const std::string Villager::Nickname()
{
	if (_customNickname.length())
		return _customNickname;
	return thePlayer.Name;
}

std::string Villager::Nickname(std::string& newNickname)
{
	const auto oldNickname = std::string(_customNickname);
	_customNickname = newNickname;
	return oldNickname;
}

void Villager::Manifest()
{
	PickOutfit();
}

void Villager::DeleteAllThings()
{
	if (HeldTool != nullptr && HeldTool->Temporary)
	{
		delete HeldTool;
		HeldTool = nullptr;
	}
	if (Hat != nullptr && Hat->Temporary)
	{
		delete Hat;
		Hat = nullptr;
	}
	if (Glasses != nullptr && Glasses->Temporary)
	{
		delete Glasses;
		Glasses = nullptr;
	}
	if (Mask != nullptr && Mask->Temporary)
	{
		delete Mask;
		Mask = nullptr;
	}
	if (Outfit != nullptr && Outfit->Temporary)
	{
		delete Outfit;
		Outfit = nullptr;
	}
}

void Villager::Depart()
{
	DeleteAllThings();
}

void Villager::PickOutfit()
{
	DeleteAllThings();

	if (Outfits.size() > 0 && std::rand() % 100 > 25)
	{
		if (Outfits.size() == 1)
		{
			Outfit = Outfits[0];
			Outfit->Temporary = false; //just to be sure.
		}
		else
		{
			for (const auto& i : Outfits)
			{
				if (std::rand() % 100 > 25)
				{
					Outfit = i;
					Outfit->Temporary = false; //just to be sure.
					break;
				}
			}
		}
	}
	else
	{
		//use default outfit
		Outfit = new InventoryItem(defaultOutfitID);
		if (!Outfit->IsOutfit())
		{
			conprint(2, "PickOutfit() for {}: \"{}\" may not exist, got a non-outfit item instead.", Name(), defaultOutfitID);
			delete Outfit;
			Outfit = new InventoryItem("psk:topsfallback");
		}
		Outfit->Temporary = true; //mark as safe to free
	}
}

bool Villager::GiveItem(InventoryItem* item)
{
	auto target = &Items;
	auto max = _maxFurnitureItems;
	if (item->IsOutfit())
	{
		target = &Outfits;
		max = _maxOutfits;
	}
	if (target->size() == max)
		return false;
	target->push_back(item);
	return true;
}

void Villager::Serialize(JSONObject& target)
{
	target["id"] = new JSONValue(ID);
	target["catchphrase"] = new JSONValue(_customCatchphrase);
	auto items = JSONArray();
	for (const auto& i : Items)
	{
		items.push_back(new JSONValue(i->FullID()));
	}
	target["items"] = new JSONValue(items);
	auto outfits = JSONArray();
	for (const auto& i : Outfits)
	{
		outfits.push_back(new JSONValue(i->FullID()));
	}
	target["outfits"] = new JSONValue(items);
}

void Villager::Deserialize(JSONObject& source)
{
	//ID is used to determine *which* Villager to deserialize *to*.
	//Still, we might do a sanity check?

	_customCatchphrase = source["catchphrase"]->AsString();
	auto items = source["items"]->AsArray();
	Items.clear();
	for (const auto& i : items)
	{
		Items.push_back(new InventoryItem(i->AsString()));
	}
	auto outfits = source["outfits"]->AsArray();
	Outfits.clear();
	for (const auto& i : outfits)
	{
		Outfits.push_back(new InventoryItem(i->AsString()));
	}
}
