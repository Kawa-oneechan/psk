#include "SpecialK.h"
#include "Town.h"
#include "Game.h"
#include "engine/InputsMap.h"
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/JSONUtils.h"

extern bool useOrthographic;

void Player::LoadModel()
{
	if (!_model)
	{
		_model = std::make_shared<::Model>("player/model.fbx");
		_hairModel = std::make_shared<::Model>(fmt::format("player/hair/{}/model.fbx", hairStyle));
	}

	_model->SetVisibility(fmt::format("Nose{:02}__mNose", noseStyle), true);

	if (Textures[0] == nullptr)
	{
		Textures[0] = new TextureArray(fmt::format("player/eyes/{}/eye*_alb.png", eyeStyle));
		Textures[1] = new TextureArray(fmt::format("player/eyes/{}/eye*_nrm.png", eyeStyle));
		Textures[2] = new TextureArray(fmt::format("player/eyes/{}/eye*_mix.png", eyeStyle));

		Textures[3] = new TextureArray(fmt::format("player/mouth/{}/mouth*_alb.png", mouthStyle));
		Textures[4] = new TextureArray(fmt::format("player/mouth/{}/mouth*_nrm.png", mouthStyle));
		Textures[5] = new TextureArray(fmt::format("player/mouth/{}/mouth*_mix.png", mouthStyle));

		Textures[6] = new TextureArray("player/cheek*_alb.png");
	}

	for (int i = 0; i < NumClothes; i++)
	{
		if (!_clothesModels[i] && _clothesItems[i])
		{
			auto& cm = _clothesModels[i];
			auto& ci = _clothesItems[i];

			cm = std::make_shared<::Model>(fmt::format("player/outfits/{}.fbx", ci->PlayerModel()));
			ClothingTextures[(i * 4) + 0] = new TextureArray(fmt::format("{}/albedo*.png", ci->Path));
			ClothingTextures[(i * 4) + 1] = new TextureArray(fmt::format("{}/normal.png", ci->Path));
			ClothingTextures[(i * 4) + 2] = new TextureArray(fmt::format("{}/mix.png", ci->Path));
			ClothingTextures[(i * 4) + 3] = new TextureArray(fmt::format("{}/opacity.png", ci->Path));
		}
	}

	if (!_clothesItems[8])
	{
		ClothingTextures[12] = new TextureArray("player/nosocks_alb.png");
		ClothingTextures[13] = new TextureArray("fallback_nrm.png");
		ClothingTextures[14] = new TextureArray("white.png");
		ClothingTextures[15] = new TextureArray("white.png");
	}
	else
	{
		//Actually load the sock textures.
	}
}

ModelP Player::Model()
{
	if (!_model)
		LoadModel();
	return _model;
}

std::string Player::Birthday()
{
	return Text::DateMD(_birthday[1], _birthday[0]);
}

int Player::findItemSlot(InventoryItemP target)
{
	if (!target)
		return NoItem;
	for (int i = 0; i < OnHandLimit; i++)
	{
		if (OnHand[i] == target)
			return i;
	}
	return NoItem;
}

bool Player::HasInventoryRoom()
{
	return std::any_of(OnHand.cbegin(), OnHand.cend(), [](InventoryItemP i) { return i; });
}

bool Player::GiveItem(InventoryItemP item)
{
	auto it = std::find(OnHand.cbegin(), OnHand.cend(), nullptr);
	if (it == OnHand.cend()) return false;
	auto i = std::distance(OnHand.cbegin(), it);
	OnHand[i] = std::move(item);
	return true;
}

void Player::SwapItems(int from, int to)
{
	if (from == NoItem || to == NoItem)
		return;
	std::swap(OnHand[from], OnHand[to]);
}

void Player::SwapItems(InventoryItemP from, InventoryItemP to)
{
	SwapItems(findItemSlot(from), findItemSlot(to));
}

bool Player::RemoveItem(int slot)
{
	if (slot == NoItem)
		return false;
	if (!OnHand[slot])
		return false;
	OnHand[slot] = nullptr;
	return true;
}

bool Player::RemoveItem(InventoryItemP item)
{
	return RemoveItem(findItemSlot(item));
}

bool Player::ConsumeItem(int slot)
{
	if (slot == NoItem)
		return false;
	return true;
}

bool Player::ConsumeItem(InventoryItemP item)
{
	return ConsumeItem(findItemSlot(item));
}

int Player::findStorageSlot(InventoryItemP target)
{
	if (!target)
		return NoItem;
	for (int i = 0; i < StorageLimit; i++)
	{
		if (Storage[i] == target)
			return i;
	}
	return NoItem;
}

bool Player::Store(int slot)
{
	if (Storage.size() >= StorageLimit)
		return false;
	Storage.push_back(OnHand[slot]);
	RemoveItem(slot);
	return true;
}

bool Player::Store(InventoryItemP item)
{
	return Store(findItemSlot(item));
}

bool Player::Retrieve(int slot)
{
	if (!HasInventoryRoom())
		return false;
	GiveItem(Storage[slot]);
	Storage.erase(Storage.begin() + slot);
	return true;
}

bool Player::Retrieve(InventoryItemP item)
{
	return Retrieve(findStorageSlot(item));
}

void Player::Draw(float dt)
{
	if (!_model)
		LoadModel();

	//TODO: Model::SetTexture method, yo!
	std::copy(&Textures[0], &Textures[3], _model->GetMesh("_mEye").Textures);
	std::copy(&Textures[3], &Textures[6], _model->GetMesh("_mMouth").Textures);
	_model->GetMesh("_mCheek").Textures[0] = Textures[6];

	_model->SetLayerByMat("_mCheek", cheeksStyle);
	_model->SetLayerByMat("_mEye", face);
	_model->SetLayerByMat("_mMouth", mouth);

	std::copy(&ClothingTextures[12], &ClothingTextures[15], _model->GetMesh("_mSocks").Textures);

	commonUniforms.PlayerSkin = SkinTone;
	commonUniforms.PlayerEyes = EyeColor;
	commonUniforms.PlayerCheeks = CheekColor;
	commonUniforms.PlayerHair = HairColor;

	_model->Draw(Position, Facing);

	if (_hairModel)
	{
		_hairModel->Draw(Position, Facing);
	}

	for (int i = 0; i < NumClothes; i++)
	{
		if (!_clothesModels[i])
			continue;
		std::copy(&ClothingTextures[(i * 4)], &ClothingTextures[(i * 4) + 4], _clothesModels[i]->GetMesh(0).Textures);
	}

	//TODO: handle hats, glasses, masks, and bags.
	//Note that certain "glasses" may cover the whole face and disable "masks".
	//Or is that vice versa?

	Person::Draw(dt);
}

bool Player::Tick(float dt)
{
	auto facing = Facing;
	auto anythingPressed = false;

	if (Inputs.Keys[(int)Binds::WalkS].State == 1)
	{
		facing = 0.0;
		if (Inputs.Keys[(int)Binds::WalkE].State == 1)
			facing = 45.0;
		else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
			facing = 315.0; //-45
		anythingPressed = true;
	}
	else if (Inputs.Keys[(int)Binds::WalkN].State == 1)
	{
		facing = 180.0;
		if (Inputs.Keys[(int)Binds::WalkE].State == 1)
			facing = 135.0;
		else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
			facing = 225.0; //-135
		anythingPressed = true;
	}
	else if (Inputs.Keys[(int)Binds::WalkE].State == 1)
	{
		facing = 90.0;
		anythingPressed = true;
	}
	else if (Inputs.Keys[(int)Binds::WalkW].State == 1)
	{
		facing = 270.0f; //-90
		anythingPressed = true;
	}

	if (anythingPressed)
	{
		Move(facing + MainCamera->Angles().z + (useOrthographic ? 0.45f : 0), dt);
	}

	if (!_model)
		LoadModel();

	//_model->MoveBone(_model->FindBone("Head"), glm::vec3(0, glm::radians(-45.0f), 0));
	//_model->MoveBone(_model->FindBone("Spine_1"), glm::vec3(sinf((float)glfwGetTime()) * glm::radians(16.0f), 0, 0));
	//_model->MoveBone(_model->FindBone("Head"), glm::vec3(glm::radians(-45.0f), 0, 0)); //look to the right (player's right)
	//_model->MoveBone(_model->FindBone("Head"), glm::vec3(0, 0, glm::radians(-45.0f))); //look up
	//_model->Bones[_model->FindBone("Head")].Rotation = glm::vec3(glm::radians(-45.0f), 0, 0); //look to the player's right
	_model->CalculateBoneTransforms();

	//TODO: make this a generic function for later.
	if (_hairModel)
	{
		auto plhBoneID = _model->FindBone("Head");
		auto& harBone = _hairModel->Bones[_hairModel->FindBone("Root")];
		harBone.LocalTransform = _model->finalBoneMatrices[plhBoneID] *
			glm::inverse(_model->Bones[plhBoneID].InverseBind);
		//put the hair upright, there's probably a reason this is required that I'm missing.
		harBone.Rotation = glm::vec3(glm::radians(-90.0f), glm::radians(-90.0f), 0);
		_hairModel->CalculateBoneTransforms();
	}

	return !anythingPressed;
}

void Player::Save()
{
	jsonValue json;
	Serialize(json);
	VFS::WriteSaveJSON("player.json", json);
}

void Player::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("player.json");
		Deserialize(json);
	}
	catch (std::runtime_error&)
	{
		conprint(1, "Couldn't load data for the player.");
	}
}

void Player::Serialize(jsonValue& target)
{
	target = json5pp::object({ { "name", "" } });

	target.as_object()["name"] = Name;
	target.as_object()["bells"] = (int)Bells;

	target.as_object()["colors"] = json5pp::object({
		{ "skin", GetJSONVec(glm::vec3(SkinTone)) },
		{ "eyes", GetJSONVec(glm::vec3(EyeColor)) },
		{ "cheek", GetJSONVec(glm::vec3(CheekColor)) },
		{ "hair", GetJSONVec(glm::vec3(HairColor))},
	});

	target.as_object()["style"] = json5pp::object({
		{ "gender", (Gender == Gender::Boy || Gender == Gender::BEnby) ? "boy" : "girl" },
		{ "eyes", eyeStyle },
		{ "mouth", mouthStyle},
		{ "cheeks", cheeksStyle },
		{ "nose", noseStyle },
		{ "hair", hairStyle },
	});

	auto items = json5pp::array({});
	for (const auto& i : OnHand)
	{
		if (i == nullptr)
			items.as_array().push_back(jsonValue(nullptr));
		else
			items.as_array().push_back(i->FullID());
	}
	target.as_object()["items"] = items;

	auto storage = json5pp::array({});
	for (const auto& i : Storage)
	{
		storage.as_array().push_back(i->FullID());
	}
	target.as_object()["storage"] = storage;

	auto outfit = json5pp::object({});
	{
		int i = 0;
		for (auto s : { "top", "bottom", "hat", "glasses", "mask", "shoes", "bag" })
		{
			outfit.as_object()[s] = _clothesItems[i] ? _clothesItems[i]->FullID() : jsonValue(nullptr);
			i++;
		}
	}
	target.as_object()["outfit"] = outfit;
}

void Player::Deserialize(jsonValue& source)
{
	auto s = source.as_object();
	Name = s["name"].as_string();
	Bells = s["bells"].as_integer();

	auto& colors = s["colors"].as_object();
	SkinTone = GetJSONColor(colors.at("skin"));
	EyeColor = GetJSONColor(colors.at("eyes"));
	CheekColor = GetJSONColor(colors.at("cheek"));
	HairColor = GetJSONColor(colors.at("hair"));

	auto& style = s["style"].as_object();
	Gender = StringToEnum<::Gender>(style.at("gender").as_string(), { "boy", "girl" });
	eyeStyle = style.at("eyes").as_integer();
	mouthStyle = style.at("mouth").as_integer();
	cheeksStyle = style.at("cheeks").as_integer();
	noseStyle = style.at("nose").as_integer();
	hairStyle = style.at("hair").as_integer();

	auto items = s["items"].as_array();
	OnHand.fill(nullptr);
	int j = 0;
	for (const auto& i : items)
	{
		if (i.is_string())
			OnHand[j] = std::make_shared<InventoryItem>(i.as_string());
		j++;
		if (j >= MaxOnHand)
			break;
	}

	auto storage = s["storage"].as_array();
	Storage.clear();
	for (const auto& i : storage)
	{
		Storage.push_back(std::make_shared<InventoryItem>(i.as_string()));
		j++;
		if (j >= MaxStorage)
			break;
	}

	auto& outfit = s["outfit"].as_object();
	{
		{
			int i = 0;
			for (const auto& c : { "top", "bottom", "hat", "glasses", "mask", "shoes", "bag" })
			{
				auto t = outfit.at(c);
				_clothesItems[i] = t.is_string() ? std::make_shared<InventoryItem>(t.as_string()) : nullptr;
				i++;
			}
		}
	}
}

Player thePlayer;
