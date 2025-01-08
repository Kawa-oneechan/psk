#include "SpecialK.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/rotate_vector.hpp"

//TODO: special characters need support for more than just tops and accessories.

Villager::Villager(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	_customModel = value["customModel"] != nullptr && value["customModel"]->IsBool() ? value["customModel"]->AsBool() : false;
	_isSpecial = value["isSpecial"] != nullptr && value["isSpecial"]->IsBool() ? value["isSpecial"]->AsBool() : false;
	_customMuzzle = (value["hasMuzzle"] != nullptr) ? value["hasMuzzle"]->AsBool() : false;
	_accessoryFixed = (value["accessoryFixed"] != nullptr) ? value["accessoryFixed"]->AsBool() : false;

	_accessoryType = (value["accessoryMapType"] != nullptr) ?
		StringToEnum<AccessoryType>(value["accessoryMapType"]->AsString(),
		{ "none", "body", "cap", "glass", "glassalpha", "bodycap" }) :
		AccessoryType::None;
	_customAccessory = (_accessoryType != AccessoryType::None && _accessoryType != AccessoryType::BodyCap);

	Textures.fill(nullptr);
	ClothingTextures.fill(nullptr);

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
		Text::Add(RefCatchphrase, *val);

	auto birthday = GetJSONDate(value["birthday"]);
	_birthday[0] = (int)birthday[0];
	_birthday[1] = (int)birthday[1];

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
	personalitySubtype = _isSpecial ? 0 : value["personalitySubtype"]->AsInteger();

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
	defaultClothingID = clothing["default"]->AsString();
	rainCoatID = (clothing["rain"]->AsArray())[0]->AsString();
	rainHatID = (clothing["rain"]->AsArray())[1]->AsString();

	face = 0;
	mouth = 0;
}

std::string Villager::Name()
{
	return Text::Get(RefName);
}

std::string Villager::Species()
{
	if (gender == Gender::Girl || gender == Gender::GEnby)
		return Text::Get(RefSpecies + ":f");
	else
		return Text::Get(RefSpecies + ":m");
}

void Villager::LoadModel()
{
	if (!_model)
	{
		if (_customModel)
			_model = std::make_shared<::Model>(fmt::format("{}/model.fbx", Path));
		else
			_model = _species->Model();

		if (_customAccessory)
			_accessoryModel = std::make_shared<::Model>(fmt::format("{}/accessory.fbx", Path));
	}

	if (Textures[0] == nullptr)
	{
		Textures[0] = new TextureArray(fmt::format("{}/body_alb.png", Path));
		Textures[1] = new TextureArray(fmt::format("{}/body_nrm.png", Path));
		Textures[2] = new TextureArray(fmt::format("{}/body_mix.png", Path));
		Textures[3] = Textures[0]; //new TextureArray(fmt::format("{}/body_alb.png", Path));
		Textures[4] = Textures[1]; //new TextureArray(fmt::format("{}/body_nrm.png", Path));
		Textures[5] = Textures[2]; //new TextureArray(fmt::format("{}/body_mix.png", Path));
		Textures[6] = new TextureArray(fmt::format("{}/eye*_alb.png", Path));
		Textures[7] = new TextureArray(fmt::format("{}/eye*_nrm.png", Path));
		Textures[8] = new TextureArray(fmt::format("{}/eye*_mix.png", Path));
		if ((_customModel && !_customMuzzle) || !_species->ModeledMuzzle)
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

			_model->SetVisibility("FaceBad__mBeak", false);
			_model->SetVisibility("FaceGood__mBeak", true);
			_model->SetVisibility("FaceNothing__mBeak", false);
		}

		if (_accessoryType == AccessoryType::Cap || _accessoryType == AccessoryType::BodyCap)
		{
			Textures[12] = new TextureArray(fmt::format("{}/cap_alb.png", Path));
			Textures[13] = new TextureArray(fmt::format("{}/cap_nrm.png", Path));
			Textures[14] = new TextureArray(fmt::format("{}/cap_mix.png", Path));
		}
		else if (_accessoryType == AccessoryType::Glass || _accessoryType == AccessoryType::GlassAlpha)
		{
			Textures[12] = new TextureArray(fmt::format("{}/glass_alb.png", Path));
			Textures[13] = new TextureArray(fmt::format("{}/glass_nrm.png", Path));
			Textures[14] = new TextureArray(fmt::format("{}/glass_mix.png", Path));
			if (_accessoryType == AccessoryType::GlassAlpha)
			{
				Textures[16] = new TextureArray(fmt::format("{}/glassalpha_alb.png", Path));
				Textures[17] = new TextureArray(fmt::format("{}/glassalpha_nrm.png", Path));
				Textures[18] = new TextureArray(fmt::format("{}/glassalpha_mix.png", Path));
				Textures[19] = new TextureArray(fmt::format("{}/glassalpha_op.png", Path));
			}
		}
	}

	if ( !_clothingModel && Clothing)
	{
		auto path = _customModel ? Path : _species->Path;
		_clothingModel = std::make_shared<::Model>(fmt::format("{}/{}.fbx", path, Clothing->Style()));

		/*
		Texture order:
		alb	nml	mix	opc
		body	0	1	2	3
		...
		*/
		ClothingTextures[0] = new TextureArray(fmt::format("{}/albedo*.png", Clothing->Path));
		ClothingTextures[1] = new TextureArray(fmt::format("{}/normal.png", Clothing->Path));
		ClothingTextures[2] = new TextureArray(fmt::format("{}/mix.png", Clothing->Path));
		ClothingTextures[3] = new TextureArray(fmt::format("{}/opacity.png", Clothing->Path));
	}
}

ModelP Villager::Model()
{
	if (!_model)
		LoadModel();
	return _model;
}

#ifdef DEBUG
void Villager::ReloadTextures()
{
	for (int i = 0; i < Textures.size(); i++)
	{
		if (Textures[i] != nullptr)
		{
			if (Textures[i]->height >= 0)
				delete Textures[i];
			Textures[i] = nullptr;
		}
	}

	LoadModel();
}
#endif

std::string Villager::Birthday()
{
	return Text::DateMD(_birthday[1], _birthday[0]);
}

std::string Villager::Catchphrase()
{
	if (memory && !memory->_customCatchphrase.empty())
		return memory->_customCatchphrase;
	return Text::Get(RefCatchphrase);
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

void Villager::SetFace(int index)
{
	face = glm::clamp(index, 0, 15);
}
void Villager::SetMouth(int index)
{
	mouth = glm::clamp(index, 0, 8);
}

void Villager::Draw(float)
{
	if (_model == nullptr)
		LoadModel();

	std::copy(&Textures[0], &Textures[2], _model->GetMesh("_mBody").Textures);
	std::copy(&Textures[0], &Textures[2], _model->GetMesh("_mCapVis").Textures);
	std::copy(&Textures[6], &Textures[8], _model->GetMesh("_mEye").Textures);
	std::copy(&Textures[9], &Textures[11], _model->GetMesh("_mMouth").Textures);
	//std::copy(&Textures[12], &Textures[14], _model->GetMesh("???").Textures);

	_model->SetLayer("_mEye", face);
	_model->SetLayer("_mMouth", mouth);
	
	_model->Draw(Position, Facing);

	if (_customAccessory && _accessoryModel)
	{
		if (_accessoryType == AccessoryType::Body)
		{
			std::copy(&Textures[0], &Textures[2], _accessoryModel->GetMesh(0).Textures);
		}
		else
		{
			std::copy(&Textures[12], &Textures[14], _accessoryModel->GetMesh(0).Textures);
			if (_accessoryType == AccessoryType::GlassAlpha)
			{
				std::copy(&Textures[16], &Textures[20], _accessoryModel->GetMesh(1).Textures);
			}
		}
		_accessoryModel->Draw(Position, Facing);
	}

	if (_clothingModel && Clothing)
	{
		std::copy(&ClothingTextures[0], &ClothingTextures[3], _clothingModel->GetMesh("_mTops").Textures);
		_clothingModel->SetLayer(Clothing->Variant());
		_clothingModel->Draw(Position, Facing);
	}
}

bool Villager::Tick(float)
{
	//TODO
	return true;
}

void Villager::Turn(float facing, float dt)
{
	auto m = Facing;
	if (m < 0) m += 360.0f;

	auto cw = facing - m;
	if (cw < 0.0) cw += 360.0f;
	auto ccw = m - facing;
	if (ccw < 0.0) ccw += 360.0f;

	constexpr auto radius = 45.0f;
	constexpr auto timeScale = 20.0f;

	auto t = (ccw < cw) ? -glm::min(radius, ccw) : glm::min(radius, cw);

	auto f = m + (t * (dt * timeScale));
	if (f < 0) f += 360.0f;

	Facing = glm::mod(f, 360.0f);
}

bool Villager::Move(float facing, float dt)
{
	Turn(facing, dt);
	
	const auto movement = glm::rotate(glm::vec2(0, 0.25f), glm::radians(Facing)) * dt;

	constexpr auto speed = 120.0f;

	//TODO: determine collisions.
	Position.x -= movement.x * speed;
	Position.z += movement.y * speed;
	return true;
}

void Villager::Manifest()
{
	if (!Icon)
		Icon = new Texture(fmt::format("{}/icon.png", Path));
		
	memory = std::make_shared<VillagerMemory>();
	try
	{
		auto json = VFS::ReadSaveJSON(fmt::format("villagers/{}.json", ID));
		Deserialize((JSONObject&)json->AsObject());
	}
	catch (std::runtime_error&)
	{
		conprint(1, "Couldn't load memories and such for {}.", ID);
	}

	PickClothing();
}

void Villager::DeleteAllThings()
{
	if (HeldTool && HeldTool->Temporary)
		HeldTool = nullptr;
	if (Cap && Cap->Temporary)
		Cap = nullptr;
	if (Glasses && Glasses->Temporary)
		Glasses = nullptr;
	if (Mask && Mask->Temporary)
		Mask = nullptr;
	if (Clothing && Clothing->Temporary)
		Clothing = nullptr;
}

void Villager::Depart()
{
	if (Icon)
	{
		delete Icon;
		Icon = nullptr;
	}
	for (int i = 0; i < Textures.size(); i++)
	{
		if (Textures[i] != nullptr)
		{
			if (Textures[i]->height >= 0)
				delete Textures[i];
			Textures[i] = nullptr;
		}
	}
	_model = nullptr;

	DeleteAllThings();



	JSONObject json;
	Serialize(json);
	auto val = JSONValue(json);
	VFS::WriteSaveJSON(fmt::format("villagers/{}.json", ID), &val);

	memory.reset();
}

void Villager::PickClothing()
{
	if (!memory)
	{
		conprint(2, "Tried to pick clothes for unmanifested villager {}. Choosing default {}.", EnName, defaultClothingID);
	}

	DeleteAllThings();

	if (memory && memory->Clothing.size() > 0 && std::rand() % 100 > 25)
	{
		if (memory->Clothing.size() == 1)
		{
			Clothing = memory->Clothing[0];
			Clothing->Temporary = false; //just to be sure.
		}
		else
		{
			for (const auto& i : memory->Clothing)
			{
				if (std::rand() % 100 > 25)
				{
					Clothing = i;
					Clothing->Temporary = false; //just to be sure.
					break;
				}
			}
		}
	}
	else if (!_customModel)
	{
		//use default clothing
		Clothing = std::make_shared<InventoryItem>(defaultClothingID);
		if (!Clothing->IsClothing())
		{
			conprint(2, "PickClothing() for {}: \"{}\" may not exist, got a non-clothing item instead.", Name(), defaultClothingID);
			Clothing = std::make_shared<InventoryItem>("psk:topsfallback");
		}
		Clothing->Temporary = true; //mark as safe to free
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
	if (item->IsClothing())
	{
		target = &memory->Clothing;
		max = _maxClothes;
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
	auto clothes = JSONArray();
	for (const auto& i : memory->Clothing)
	{
		clothes.push_back(new JSONValue(i->FullID()));
	}
	target["clothing"] = new JSONValue(items);
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
	auto clothes = source["clothing"]->AsArray();
	memory->Clothing.clear();
	for (const auto& i : clothes)
	{
		memory->Clothing.push_back(std::make_shared<InventoryItem>(i->AsString()));
	}
}
