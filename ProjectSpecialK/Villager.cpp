#include "SpecialK.h"

Villager::Villager(JSONObject& value) : NameableThing(value)
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

	_customModel = value["customModel"] != nullptr && value["customModel"]->IsBool() ? value["customModel"]->AsBool() : false;
	_isSpecial = value["isSpecial"] != nullptr && value["isSpecial"]->IsBool() ? value["isSpecial"]->AsBool() : false;
	
	if (_isSpecial)
	{
		//_species = nullptr;
		RefSpecies = "<<special>>";
	}
	else
	{
		auto sp = value["species"]->AsString();
		auto refType = Database::RefType::Enumeration;
		_species = Database::Find<::Species>(sp.c_str(), &species, &refType);
		if (refType != Database::RefType::Enumeration)
			throw std::runtime_error(fmt::format("Species \"{0}\" is not an $enumeration, should probably be \"${0}\", while loading {1}.", sp, ID).c_str());
		else if (_species == nullptr)
			throw std::runtime_error(fmt::format("Unknown species \"{}\" while loading {}.", sp, ID).c_str());
		RefSpecies = fmt::format("species:{}", _species->ID);
		StringToLower(RefSpecies);
	}
	_model = nullptr;

/*
	auto ref = fmt::format("name:{}", ID.substr(ID.find_last_of(':') + 1));
	StringToLower(ref);
	StripSpaces(ref);
	RefName = ref;

	auto val = value.find("name")->second;
	TextAdd(ref, *val);

	EnName = TextGet(ref, Language::EUen);
*/

	//Normally, special villagers have no catchphrase but we'll allow it as an option.
	if (_isSpecial && value["catchphrase"] == nullptr)
	{
		RefCatchphrase = "<<special>>";
	}
	else
	{
		RefCatchphrase = fmt::format("catchphrase:{}", ID.substr(ID.find_last_of(':') + 1));
		auto val = value["catchphrase"];
		TextAdd(RefCatchphrase, *val);
	}

	auto birthday = value["birthday"]->AsArray();
	_birthday[0] = (int)birthday[0]->AsNumber();
	_birthday[1] = (int)birthday[1]->AsNumber();

	auto _gender = value["gender"];
	if (_gender != nullptr)
	{
		if (_gender->AsString() == "$mas")
			this->gender = Gender::Boy;
		else if (_gender->AsString() == "$fem")
			this->gender = Gender::Girl;
		else if (_gender->AsString() == "$mnb")
			this->gender = Gender::BEnby;
		else if (_gender->AsString() == "$fnb")
			this->gender = Gender::GEnby;
		else
			throw std::runtime_error(fmt::format("Unknown gender \"{}\" while loading {}.", _gender->AsString(), ID).c_str());
	}

	auto nametag = value["nameTag"]->AsArray();
	NameTag[0] = nametag[0]->AsString();
	NameTag[1] = nametag[1]->AsString();

	if (_isSpecial)
		personality = nullptr;
	else
	{
		auto refType = Database::RefType::Enumeration;
		personality = Database::Find<::Personality>(value["personality"]->AsString(), &personalities, &refType);
		if (refType != Database::RefType::Enumeration)
			throw std::runtime_error(fmt::format("Personality \"{0}\" is not an $enumeration, should probably be \"${0}\", while loading {1}.", value["personality"]->AsString(), ID).c_str());
		else if (personality == nullptr)
			throw std::runtime_error(fmt::format("Unknown personality \"{}\" while loading {}.", value["personality"]->AsString(), ID).c_str());
	}
	personalitySubtype = _isSpecial ? 0 : (int)value["personalitySubtype"]->AsNumber();
	//hobby = value["hobby"];
	{
		auto refType = Database::RefType::Enumeration;
		hobby = Database::Find<::Hobby>(value["hobby"]->AsString(), &hobbies, &refType);
		if (refType != Database::RefType::Enumeration)
			fmt::print("\x1B[93m" "Hobby \"{0}\" is not an $enumeration, should probably be \"${0}\", while loading {1}.\n" "\x1B[0m", value["hobby"]->AsString(), ID);
		else if (hobby == nullptr)
		{
			fmt::print("\x1B[93m" "Unknown hobby \"{}\" while loading {}.\n" "\x1B[0m", value["hobby"]->AsString(), ID);
			hobby = Database::Find<::Hobby>("$fallback", &hobbies, &refType);
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

std::string Villager::Catchphrase()
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

std::string Villager::Nickname()
{
	if (_customNickname.length())
		return _customNickname;
	return  "Kawa"; //playerName;
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

	if (GivenItems.size() > 0 && std::rand() % 100 > 25)
	{
		if (GivenItems.size() == 1)
		{
			Outfit = GivenItems[0];
			Outfit->Temporary = false; //just to be sure.
		}
		else
		{
			for (const auto& i : GivenItems)
			{
				if (i->IsOutfit() && std::rand() % 100 > 25)
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
		Outfit->Temporary = true; //mark as safe to free
	}

}

void Villager::Serialize(JSONObject& target)
{
	target["id"] = new JSONValue(ID);
	target["catchphrase"] = new JSONValue(_customCatchphrase);
	auto givenItems = JSONArray();
	for (const auto& i : GivenItems)
	{
		givenItems.push_back(new JSONValue(i->FullID()));
	}
	target["givenItems"] = new JSONValue(givenItems);
}

void Villager::Deserialize(JSONObject& source)
{
	//ID is used to determine *which* Villager to deserialize *to*.
	//Still, we might do a sanity check?

	_customCatchphrase = source["catchphrase"]->AsString();
	auto givenItems = source["givenItems"]->AsArray();
	GivenItems.clear();
	for (const auto& i : givenItems)
	{
		GivenItems.push_back(new InventoryItem(i->AsString()));
		//GivenItems.push_back(ResolveItem(i->AsString(), "toolfallback"));
	}
}