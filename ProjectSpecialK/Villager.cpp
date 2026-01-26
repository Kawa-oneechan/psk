#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/JSONUtils.h"
#include "engine/Random.h"
#include "engine/Console.h"
#include "Villager.h"
#include "Database.h"
#include "Town.h"
#include "Animator.h"
#include "Player.h"
#include "Utilities.h"
#include "DialogueBox.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

static SpeciesP specialDummy;

//TODO: special characters need support for more than just tops and accessories.
//IDEA: use a generic Clothing array instead of a single model.
//Lets do that yeah.

Villager::Villager(jsonObject& value, const std::string& filename) : NameableThing(value, filename)
{
	_customModel = value["customModel"].is_boolean() ? value["customModel"].as_boolean() : false;
	_isSpecial = value["isSpecial"].is_boolean() ? value["isSpecial"].as_boolean() : false;
	_customMuzzle = value["hasMuzzle"].is_boolean() ? value["hasMuzzle"].as_boolean() : false;
	_accessoryFixed = value["accessoryFixed"].is_boolean() ? value["accessoryFixed"].as_boolean() : false;

	_accessoryType = (value["accessoryMapType"].is_string()) ?
		StringToEnum<AccessoryType>(value["accessoryMapType"].as_string(),
		{ "none", "body", "cap", "glass", "glassalpha", "bodycap" }) :
		AccessoryType::None;
	_customAccessory = (_accessoryType != AccessoryType::None && _accessoryType != AccessoryType::BodyCap);

	Textures.fill(nullptr);
	ClothingTextures.fill(nullptr);

	if (!_isSpecial)
	{
		auto sp = value["species"];
		_species = Database::FindEx<::Species>(sp, species);
		if (!_species)
			throw std::runtime_error(fmt::format("Unknown species {} while loading {}.", sp.stringify5(), ID));
	}
	_model = nullptr;

	//Normally, special villagers have no catchphrase but we'll allow it as an option.
	RefCatchphrase = fmt::format("catchphrase:{}", ID);
	auto val = value["catchphrase"];
	if (!val || (val.is_string() && val.as_string().empty()))
		val = "dummy";
	if (val.is_string() && val.as_string()[0] == '#')
		RefCatchphrase = val.as_string().substr(1);
	else
		Text::Add(RefCatchphrase, val);

	auto birthday = GetJSONDate(value["birthday"]);
	_birthday[0] = (int)birthday[0];
	_birthday[1] = (int)birthday[1];

	auto _gender = value["gender"];
	if (_gender.is_string())
	{
		try
		{
			this->Gender = StringToEnum<::Gender>(_gender.as_string(), { "boy", "girl", "enby-b", "enby-g" });
		}
		catch (std::range_error& re)
		{
			throw std::runtime_error(fmt::format("Unknown gender {} while loading {}: {}", _gender.stringify5(), ID, re.what()));
		}
	}

	auto nametag = value["nameTag"].as_array();
	for (int i = 0; i < 2; i++)
	{
		NameTag[i] = GetJSONColor(nametag[i]);
		if (NameTag[i].a == -1)
			throw std::runtime_error(fmt::format("Not a well-formed color value {} while loading {}.", nametag[i].stringify5(), ID));
	}

	if (_isSpecial)
		personality = nullptr;
	else
	{
		personality = Database::FindEx<::Personality>(value["personality"], personalities);
		if (!personality)
			throw std::runtime_error(fmt::format("Unknown personality {} while loading {}.", value["personality"].stringify5(), ID));
	}
	personalitySubtype = _isSpecial ? 0 : value["personalitySubtype"].as_integer();

	{
		hobby = Database::Find<::Hobby>(value["hobby"].as_string(), hobbies);
		if (!hobby)
		{
			conprint(1, "Unknown hobby {} while loading {}.", value["hobby"].stringify5(), ID);
			hobby = Database::Find<::Hobby>("fallback", hobbies);
		}
	}

	if (value["umbrella"].is_string()) umbrellaID = value["umbrella"].as_string();
	if (value["photo"].is_string()) photoID = value["photo"].as_string();
	if (value["poster"].is_string()) portraitID = value["poster"].as_string();

	//Everything after this point is only for regular villagers.
	if (_isSpecial)
		return;

	auto clothing = value["clothing"].as_object();
	defaultClothingID = clothing["default"].as_string();
	rainCoatID = (clothing["rain"].as_array())[0].as_string();
	rainHatID = (clothing["rain"].as_array())[1].as_string();

	face = 0;
	mouth = 0;
}

Species* Villager::Species()
{
	if (!_species)
	{
		if (!specialDummy)
		{
			jsonObject temp;
			temp["id"] = "__special__";
			temp["name"] = "<special>";
			specialDummy = std::make_shared<::Species>(temp, "");
		}
		return specialDummy.get();
	}
	return _species.get();
};

std::string Villager::SpeciesName()
{
	if (!_species)
		return "<special>";
	auto ref = fmt::format("name:{}:{}", _species->ID,
		(Gender == Gender::Girl || Gender == Gender::GEnby) ?
		'f' : 'm');
	return Text::Get(ref);
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

	for (int i = 0; i < LastRegularClothingSlot; i++)
	{
		if (!_clothesModels[i] && _clothesItems[i])
		{
			auto& cm = _clothesModels[i];
			auto& ci = _clothesItems[i];

			auto path = (_customModel && _isSpecial) ? Path : _species->Path;
			auto modelFile = fmt::format("{}/{}.fbx", path, ci->Style());
			if (ci->Style() == "")
				modelFile = fmt::format("{}/model.fbx", ci->Path);
			cm = std::make_shared<::Model>(modelFile);
			ClothingTextures[(i * 4) + 0] = new TextureArray(fmt::format("{}/albedo*.png", ci->Path));
			ClothingTextures[(i * 4) + 1] = new TextureArray(fmt::format("{}/normal*.png", ci->Path));
			ClothingTextures[(i * 4) + 2] = new TextureArray(fmt::format("{}/mix*.png", ci->Path));
			ClothingTextures[(i * 4) + 3] = new TextureArray(fmt::format("{}/opacity*.png", ci->Path));
		}
	}

	try
	{
		auto dummy = _model->GetMesh("mCapVis");
	}
	catch (std::runtime_error)
	{
		_hasCapViz = false;
	}

	if (animator == nullptr)
		animator = std::make_unique<::Animator>(_model->Bones);
	animator->APose();
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

void Villager::Draw(float dt)
{
	const auto cullMargin = 192;

	glm::vec2 proj;
	Project(Position, proj);
	if (proj.x < -cullMargin || proj.x > width + cullMargin)
		return;

	if (_model == nullptr)
		LoadModel();

	std::copy(&Textures[0], &Textures[2], _model->GetMesh("mBody").Textures);
	if (_hasCapViz)
	{
		if (_accessoryType != AccessoryType::BodyCap)
			std::copy(&Textures[0], &Textures[2], _model->GetMesh("mCapVis").Textures);
		else
			std::copy(&Textures[12], &Textures[14], _model->GetMesh("mCapVis").Textures);
	}
	std::copy(&Textures[6], &Textures[8], _model->GetMesh("mEye").Textures);
	if ((_customModel && !_customMuzzle) || !_species->ModeledMuzzle)
	{
		std::copy(&Textures[9], &Textures[11], _model->GetMesh("mMouth").Textures);
		_model->SetLayerByMat("mMouth", mouth);
	}
	else
	{
		static const std::string meshes[] = { "FaceNothing__mBeak", "FaceGood__mBeak", "FaceBad__mBeak" };
		for (int i = 0; i < 3; i++)
		{
			std::copy(&Textures[9], &Textures[11], _model->GetMesh(meshes[i]).Textures);
			_model->SetVisibility(meshes[i], mouth / 3 == i);
		}
		//std::copy(&Textures[9], &Textures[11], _model->GetMesh("FaceBad__mBeak").Textures);
		//std::copy(&Textures[9], &Textures[11], _model->GetMesh("FaceGood__mBeak").Textures);
		//std::copy(&Textures[9], &Textures[11], _model->GetMesh("FaceNothing__mBeak").Textures);
	}
	//std::copy(&Textures[12], &Textures[14], _model->GetMesh("???").Textures);

	_model->SetLayerByMat("mEye", face);

	//_model->Draw(Position, Facing);
	_model->Draw();

	if (_customAccessory && _accessoryModel)
	{
		if (_accessoryType == AccessoryType::Body)
		{
			for (auto& m : _accessoryModel->Meshes)
				std::copy(&Textures[0], &Textures[2], m.Textures);
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

	for (int i = 0; i < LastRegularClothingSlot; i++)
	{
		if (!_clothesModels[i])
			continue;
		std::copy(&ClothingTextures[(i * 4)], &ClothingTextures[(i * 4) + 4], _clothesModels[i]->GetMesh(0).Textures);
	}

	Person::Draw(dt);

	MeshBucket::Flush();
}

bool Villager::Tick(float)
{
	if (!_model)
		LoadModel();

	//TODO: update animator
	animator->CopyBones(_model);
	auto& root = _model->Bones[_model->FindBone("Root")];
	root.Translation = Position;
	root.Rotation = glm::vec3(0, glm::radians(Facing), 0);;

	if ((_customModel && _customMuzzle) || (_species && _species->ModeledMuzzle))
	{
		_model->SetVisibility("FaceBad__mBeak", mouth >= 3 && mouth < 6);
		_model->SetVisibility("FaceGood__mBeak", mouth >= 6);
		_model->SetVisibility("FaceNothing__mBeak", mouth < 3);
		static const float mouthPoses[] = { 0.0f, 0.15f, 0.40f };
		const auto mouthBone = _model->FindBone("Mouth");
		_model->Bones[mouthBone].Rotation.z = mouthPoses[mouth % 3];
	}

	_model->CalculateBoneTransforms();

	if (scriptRunner && scriptRunner->Runnable() && !Mutex)
	{
		scriptRunner->Call();
		if (scriptRunner->Status() == sol::call_status::ok) //not yielded
		{
			//somehow clean up properly?
			scriptRunner.reset();
		}
	}

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
		Deserialize(json.as_object());
	}
	catch (std::runtime_error&)
	{
		conprint(1, "Couldn't load memories and such for {}.", ID);
	}

	PickClothing();
}

void Villager::DeleteAllThings()
{
	for (int i = 0; i < LastRegularClothingSlot; i++)
	{
		if (_clothesItems[i] && _clothesItems[i]->Temporary)
			_clothesItems[i] = nullptr;
	}
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
	animator = nullptr;

	DeleteAllThings();



	jsonValue json = json5pp::object({});
	Serialize(json.as_object());
	VFS::WriteSaveJSON(fmt::format("villagers/{}.json", ID), json);

	memory.reset();
}

int Villager::PickSNPCOutfit(sol::object a)
{
	if (!_isSpecial)
		return 0; //Not special

	auto json = VFS::ReadJSON(fmt::format("{}/{}", Path, File));
	auto value = json.as_object();
	if (value["outfits"].is_array())
	{
		auto fits = value["outfits"].as_array();
		jsonObject fit;

		if (a.is<std::string>())
		{
			auto target = a.as<std::string>();
			for (auto& _f : fits)
			{
				if (_f.as_object()["id"].as_string() == target)
				{
					fit = _f.as_object();
					break;
				}
			}
		}
		else if (a.is<int>())
		{
			auto ordinal = a.as<int>();
			if (ordinal < 0 || ordinal >= fits.size())
				return 2; //Not found
			fit = fits[a.as<int>()].as_object();
		}

		if (fit.empty())
			return 2; //Not found

		debprint(0, "Assigning outfit {} to {}.", fit["id"].as_string(), ID);
		if (fit["tops"].is_string())
			_clothesItems[0] = std::make_shared<InventoryItem>(fit["tops"].as_string());
		if (fit["bottoms"].is_string())
			_clothesItems[1] = std::make_shared<InventoryItem>(fit["bottoms"].as_string());
		return 3; //Success!
	}

	return 1; //No outfits
}

void Villager::PickClothing()
{
	if (!memory)
	{
		conprint(2, "Tried to pick clothes for unmanifested villager {}. Choosing default {}.", EnName, defaultClothingID);
	}

	DeleteAllThings();

	if (_isSpecial)
	{
		auto script = VFS::ReadString(fmt::format("{}/spawn.lua", Path));
		if (!script.empty())
		{
			Sol["me"] = Database::Find(Hash, villagers);
			Sol.do_string(script);
		}
		else
			PickSNPCOutfit(0);
		return;
	}

	if (memory && memory->Clothing.size() > 0 && Random::GetFloat() > 25)
	{
		if (memory->Clothing.size() == 1)
		{
			_clothesItems[0] = memory->Clothing[0];
			_clothesItems[0]->Temporary = false; //just to be sure.
		}
		else
		{
			for (const auto& i : memory->Clothing)
			{
				if (Random::GetFloat() > 25)
				{
					_clothesItems[0] = i;
					_clothesItems[0]->Temporary = false; //just to be sure.
					break;
				}
			}
		}
	}
	else if (!(_customModel && _isSpecial))
	{
		//use default clothing
		_clothesItems[0] = std::make_shared<InventoryItem>(defaultClothingID);
		if (!_clothesItems[0]->IsClothing())
		{
			conprint(2, "PickClothing() for {}: \"{}\" may not exist, got a non-clothing item instead.", Name(), defaultClothingID);
			_clothesItems[0] = std::make_shared<InventoryItem>("psk:topsfallback");
		}
		_clothesItems[0]->Temporary = true; //mark as safe to free
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

void Villager::Serialize(jsonObject& target)
{
	if (!memory)
	{
		conprint(2, "Tried to serialize unmanifested villager {}.", EnName);
		return;
	}

	target["id"] = ID;
	target["catchphrase"] = memory->_customCatchphrase;
	auto myItems = json5pp::array({});
	for (const auto& i : memory->Items)
	{
		myItems.as_array().push_back(i->FullID());
	}
	target["items"] = std::move(myItems);
	auto clothes = json5pp::array({});
	for (const auto& i : memory->Clothing)
	{
		clothes.as_array().push_back(i->FullID());
	}
	target["clothing"] = std::move(clothes);
}

void Villager::Deserialize(jsonObject& source)
{
	//ID is used to determine *which* Villager to deserialize *to*.
	//Still, we might do a sanity check?

	if (!memory)
	{
		conprint(2, "Tried to deserialize unmanifested villager {}.", EnName);
		return;
	}

	memory->_customCatchphrase = source["catchphrase"].as_string();
	auto myItems = source["items"].as_array();
	memory->Items.clear();
	for (const auto& i : myItems)
	{
		memory->Items.push_back(std::make_shared<InventoryItem>(i.as_string()));
	}
	auto clothes = source["clothing"].as_array();
	memory->Clothing.clear();
	for (const auto& i : clothes)
	{
		memory->Clothing.push_back(std::make_shared<InventoryItem>(i.as_string()));
	}
}

void Villager::TestScript()
{
	/*
	Sol.do_string(R"SOL(

	function start()
		dialogue("Panting and sweating...")
		dialogue("Are we done yet?")
		-- dialogue("Save complete.", 4)
		-- dialogue() --oops
	end
	)SOL");
	runningScript = std::make_shared<sol::coroutine>(Sol["start"]);
	dlgBox->Mutex = &testMutex;
	testMutex = false;
	console->visible = false;
	*/

	auto testScript = R"SOL(

	function start()
		dialogue("Panting and sweating...")
		dialogue("Are we done yet?")
		-- dialogue("Save complete.", 4)
		-- dialogue() --oops
	end

	)SOL";

	console->visible = false;
	Mutex = false;
	scriptRunner = std::make_shared<ScriptRunner>("start", testScript, &Mutex);
	dlgBox->Mutex = scriptRunner->Mutex;
}

//TODO: split this into its own files
namespace SolBinds
{
	extern void Setup(sol::state& sol);
}
ScriptRunner::ScriptRunner(const std::string& entryPoint, const std::string& script, bool* mutex)
{
	SolBinds::Setup(myState);
	Mutex = mutex;
	myState.do_string(script);
	currentCoro = std::make_shared<sol::coroutine>(myState[entryPoint]);
}
ScriptRunner::~ScriptRunner()
{
	currentCoro.reset();
}
