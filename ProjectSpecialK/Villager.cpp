#include "SpecialK.h"

Villager::Villager(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	_customModel = value["customModel"] != nullptr && value["customModel"]->IsBool() ? value["customModel"]->AsBool() : false;
	_isSpecial = value["isSpecial"] != nullptr && value["isSpecial"]->IsBool() ? value["isSpecial"]->AsBool() : false;
	
	Textures.fill(nullptr);

	if (_isSpecial)
	{
		RefSpecies = "<<special>>";
	}
	else
	{
		auto sp = value["species"];
		_species = Database::Find<::Species>(sp, species);
		if (!_species)
			throw std::runtime_error(fmt::format("Unknown species {} while loading {}.", sp->Stringify(), ID));
		RefSpecies = fmt::format("name:{}", _species->ID);
		StringToLower(RefSpecies);
	}
	_model = nullptr;

	//Normally, special villagers have no catchphrase but we'll allow it as an option.
	RefCatchphrase = fmt::format("catchphrase:{}", ID);
	auto val = value["catchphrase"];
	if (!val || (val->IsString() && val->AsString().empty()))
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
		_birthday[0] = glm::clamp((int)_birthday[0], 1, 31); //mind you, half the months don't go that high
		_birthday[1] = glm::clamp((int)_birthday[1], 1, 12);
	}

	auto _gender = value["gender"];
	if (_gender != nullptr)
	{
		try
		{
			this->gender = StringToEnum<Gender>(_gender->AsString(), { "boy", "girl", "enby-b", "enby-g" });
		}
		catch (std::range_error& re)
		{
			throw std::runtime_error(fmt::format("Unknown gender {} while loading {}: {}", _gender->Stringify(), ID, re.what()));
		}
	}

	auto nametag = value["nameTag"]->AsArray();
	for (int i = 0; i < 2; i++)
	{
		NameTag[i] = GetJSONColor(nametag[i]);
		if (NameTag[i].a == -1)
			throw std::runtime_error(fmt::format("Not a well-formed color value {} while loading {}.", nametag[i]->Stringify(), ID));
	}

	if (_isSpecial)
		personality = nullptr;
	else
	{
		personality = Database::Find<::Personality>(value["personality"], personalities);
		if (!personality)
			throw std::runtime_error(fmt::format("Unknown personality {} while loading {}.", value["personality"]->Stringify(), ID));
	}
	personalitySubtype = _isSpecial ? 0 : (int)value["personalitySubtype"]->AsNumber();

	{
		hobby = Database::Find<::Hobby>(value["hobby"], hobbies);
		if (!hobby)
		{
			conprint(1, "Unknown hobby {} while loading {}.", value["hobby"]->Stringify(), ID);
			hobby = Database::Find<::Hobby>("fallback", hobbies);
		}
	}

	if (value["umbrella"] != nullptr) umbrellaID = value["umbrella"]->AsString();
	if (value["photo"] != nullptr) photoID = value["photo"]->AsString();
	if (value["poster"] != nullptr) portraitID = value["poster"]->AsString();

	//Everything after this point is only for regular villagers.
	if (_isSpecial)
		return;

	auto clothing = value["clothing"]->AsObject();
	defaultOutfitID = clothing["default"]->AsString();
	rainCoatID = (clothing["rain"]->AsArray())[0]->AsString();
	rainHatID = (clothing["rain"]->AsArray())[1]->AsString();
}

std::string Villager::Name()
{
	return TextGet(RefName);
}

std::string Villager::Species()
{
	if (gender == Gender::Girl || gender == Gender::GEnby)
		return TextGet(RefSpecies + ":f");
	else
		return TextGet(RefSpecies + ":m");
}

void Villager::LoadModel()
{
	if (!_model)
	{
		if (_customModel)
		{
		}
		else
		{
			_model = _species->Model();
		}
	}

	if (Textures[0] == nullptr)
	{
		/*
		Texture order:
				alb	nml	mix
		body	0	1	2
		capvis	3	4	5
		eyes	6	7	8
		mouth	9	10	11
		accs.	12	13	14
		...
		*/
		Textures[0] = new TextureArray(fmt::format("{}/body_alb.png", Path));
		Textures[1] = new TextureArray(fmt::format("{}/body_nrm.png", Path));
		Textures[2] = new TextureArray(fmt::format("{}/body_mix.png", Path));
		Textures[3] = Textures[0]; //new TextureArray(fmt::format("{}/body_alb.png", Path));
		Textures[4] = Textures[1]; //new TextureArray(fmt::format("{}/body_nrm.png", Path));
		Textures[5] = Textures[2]; //new TextureArray(fmt::format("{}/body_mix.png", Path));
		Textures[6] = new TextureArray(fmt::format("{}/eye*_alb.png", Path));
		Textures[7] = new TextureArray(fmt::format("{}/eye*_nrm.png", Path));
		Textures[8] = new TextureArray(fmt::format("{}/eye*_mix.png", Path));
		if (!_species->ModeledMuzzle)
		{
			Textures[9] = new TextureArray(fmt::format("{}/mouth*_alb.png", Path));
			Textures[10] = new TextureArray(fmt::format("{}/mouth*_nrm.png", Path));
			Textures[11] = new TextureArray(fmt::format("{}/mouth*_mix.png", Path));
		}
		else
		{
			Textures[9] = new TextureArray(fmt::format("{}/beak_alb.png", Path));
			Textures[10] = new TextureArray(fmt::format("{}/beak_nrm.png", Path));
			Textures[11] = new TextureArray(fmt::format("{}/beak_mix.png", Path));

			_model->GetMesh("FaceBad__mBeak").Visible = false;
			_model->GetMesh("FaceGood__mBeak").Visible = true;
			_model->GetMesh("FaceNothing__mBeak").Visible = false;
		}
		//TODO: detect and load accessories, which take their own sub-model.
	}

	/*
	Load outfit parts too. Note that we cannot trust InventoryItem::LoadModel here
	because outfits are tailored to the character. So we must load our own outfit
	models.
	*/
	if (!_outfitModel && Outfit)
	{
		auto style = "ts_short";
		_outfitModel = std::make_shared<::Model>(fmt::format("{}/{}.fbx", _species->Path, style));

		Outfit->LoadTextures();
	}
}

ModelP Villager::Model()
{
	if (!_model)
		LoadModel();
	return _model;
}

std::string Villager::Birthday()
{
	return TextDateMD(_birthday[1], _birthday[0]);
}

std::string Villager::Catchphrase()
{
	if (memory && !memory->_customCatchphrase.empty())
		return memory->_customCatchphrase;
	return TextGet(RefCatchphrase);
}

std::string Villager::Catchphrase(const std::string& newPhrase)
{
	if (!memory)
	{
		conprint(2, "Tried to set catchphrase for unmanifested villager {}.", EnName);
		return "burp";
	}
	const auto oldPhrase = std::string(memory->_customCatchphrase);
	memory->_customCatchphrase = newPhrase;
	return oldPhrase;
}

std::string Villager::Nickname()
{
	if (memory && !memory->_customNickname.empty())
		return memory->_customNickname;
	return thePlayer.Name;
}

std::string Villager::Nickname(const std::string& newNickname)
{
	if (!memory)
	{
		conprint(2, "Tried to set nickanme for unmanifested villager {}.", EnName);
		return "Bob";
	}
	const auto oldNickname = std::string(memory->_customNickname);
	memory->_customNickname = newNickname;
	return oldNickname;
}

extern Shader* modelShader;
void Villager::Draw(double)
{
	if (_model == nullptr)
		LoadModel();

	modelShader->Use();

	_model->Textures[0] = Textures[0];
	_model->Textures[1] = Textures[1];
	_model->Textures[2] = Textures[2];
	_model->Textures[4] = Textures[3];
	_model->Textures[5] = Textures[4];
	_model->Textures[6] = Textures[5];
	_model->Textures[8] = Textures[6];
	_model->Textures[9] = Textures[7];
	_model->Textures[10] = Textures[8];
	_model->Textures[12] = Textures[9];
	_model->Textures[13] = Textures[10];
	_model->Textures[14] = Textures[11];

	_model->Draw();

	if (_outfitModel && Outfit)
	{
		//TODO: have Outfit handle its own drawing.
		Outfit->AssignTextures(_outfitModel);
		_outfitModel->TexArrayLayers[0] = 0; //variant test
		_outfitModel->Draw();
	}
}

void Villager::Manifest()
{
	memory = std::make_shared<VillagerMemory>();
	//TODO: grab memories from savegame (can't use VFS here)
	//using Deserialize.
	try
	{
		auto json = VFS::ReadSaveJSON(fmt::format("villagers/{}.json", ID));
		Deserialize((JSONObject&)json->AsObject());
	}
	catch (std::runtime_error&)
	{
		conprint(1, "Couldn't load memories and such for {}.", ID);
	}

	PickOutfit();
}

void Villager::DeleteAllThings()
{
	if (HeldTool && HeldTool->Temporary)
		HeldTool = nullptr;
	if (Hat && Hat->Temporary)
		Hat = nullptr;
	if (Glasses && Glasses->Temporary)
		Glasses = nullptr;
	if (Mask && Mask->Temporary)
		Mask = nullptr;
	if (Outfit && Outfit->Temporary)
		Outfit = nullptr;
}

void Villager::Depart()
{
	DeleteAllThings();

	//TODO: store memories to savegame (can't use VFS here)
	//using Serialize.
	JSONObject json;
	Serialize(json);
	auto val = JSONValue(json);
	VFS::WriteSaveJSON(fmt::format("villagers/{}.json", ID), &val);

	memory.reset();
}

void Villager::PickOutfit()
{
	if (!memory)
	{
		conprint(2, "Tried to pick outfit for unmanifested villager {}. Choosing default {}.", EnName, defaultOutfitID);
	}

	DeleteAllThings();

	if (memory && memory->Outfits.size() > 0 && std::rand() % 100 > 25)
	{
		if (memory->Outfits.size() == 1)
		{
			Outfit = memory->Outfits[0];
			Outfit->Temporary = false; //just to be sure.
		}
		else
		{
			for (const auto& i : memory->Outfits)
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
		Outfit = std::make_shared<InventoryItem>(defaultOutfitID);
		if (!Outfit->IsOutfit())
		{
			conprint(2, "PickOutfit() for {}: \"{}\" may not exist, got a non-outfit item instead.", Name(), defaultOutfitID);
			Outfit = std::make_shared<InventoryItem>("psk:topsfallback");
		}
		Outfit->Temporary = true; //mark as safe to free
	}
}

bool Villager::GiveItem(InventoryItemP item)
{
	if (!memory)
	{
		conprint(2, "Tried to give item to unmanifested villager {}.", EnName);
		return false;
	}

	auto target = &memory->Items;
	auto max = _maxFurnitureItems;
	if (item->IsOutfit())
	{
		target = &memory->Outfits;
		max = _maxOutfits;
	}
	if (target->size() == max)
		return false;
	target->push_back(item);
	return true;
}

void Villager::Serialize(JSONObject& target)
{
	if (!memory)
	{
		conprint(2, "Tried to serialize unmanifested villager {}.", EnName);
		return;
	}

	target["id"] = new JSONValue(ID);
	target["catchphrase"] = new JSONValue(memory->_customCatchphrase);
	auto items = JSONArray();
	for (const auto& i : memory->Items)
	{
		items.push_back(new JSONValue(i->FullID()));
	}
	target["items"] = new JSONValue(items);
	auto outfits = JSONArray();
	for (const auto& i : memory->Outfits)
	{
		outfits.push_back(new JSONValue(i->FullID()));
	}
	target["outfits"] = new JSONValue(items);
}

void Villager::Deserialize(JSONObject& source)
{
	//ID is used to determine *which* Villager to deserialize *to*.
	//Still, we might do a sanity check?

	if (!memory)
	{
		conprint(2, "Tried to deserialize unmanifested villager {}.", EnName);
		return;
	}

	memory->_customCatchphrase = source["catchphrase"]->AsString();
	auto items = source["items"]->AsArray();
	memory->Items.clear();
	for (const auto& i : items)
	{
		memory->Items.push_back(std::make_shared<InventoryItem>(i->AsString()));
	}
	auto outfits = source["outfits"]->AsArray();
	memory->Outfits.clear();
	for (const auto& i : outfits)
	{
		memory->Outfits.push_back(std::make_shared<InventoryItem>(i->AsString()));
	}
}
