#include "SpecialK.h"
#include "Town.h"
#include "Game.h"
#include "engine/InputsMap.h"
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
	JSONObject json;
	Serialize(json);
	auto val = JSONValue(json);
	VFS::WriteSaveJSON("player.json", &val);
}

void Player::Load()
{
	try
	{
		auto json = VFS::ReadSaveJSON("player.json");
		Deserialize((JSONObject&)json->AsObject());
	}
	catch (std::runtime_error&)
	{
		conprint(1, "Couldn't load data for the player.");
	}
}

void Player::Serialize(JSONObject& target)
{
	target["name"] = new JSONValue(Name);
	target["bells"] = new JSONValue((int)Bells);

	JSONObject colors;
	colors["skin"] = GetJSONVec(glm::vec3(SkinTone));
	colors["eyes"] = GetJSONVec(glm::vec3(EyeColor));
	colors["cheek"] = GetJSONVec(glm::vec3(CheekColor));
	colors["hair"] = GetJSONVec(glm::vec3(HairColor));
	target["colors"] = new JSONValue(colors);

	JSONObject style;
	style["gender"] = new JSONValue((Gender == Gender::Boy || Gender == Gender::BEnby) ? "boy" : "girl");
	style["eyes"] = new JSONValue(eyeStyle);
	style["mouth"] = new JSONValue(mouthStyle);
	style["cheeks"] = new JSONValue(cheeksStyle);
	style["nose"] = new JSONValue(noseStyle);
	style["hair"] = new JSONValue(hairStyle);
	target["style"] = new JSONValue(style);

	JSONArray items;
	for (const auto& i : OnHand)
	{
		if (i == nullptr)
			items.push_back(new JSONValue());
		else
			items.push_back(new JSONValue(i->FullID()));
	}
	target["items"] = new JSONValue(items);

	JSONArray storage;
	for (const auto& i : Storage)
	{
		storage.push_back(new JSONValue(i->FullID()));
	}
	target["storage"] = new JSONValue(storage);

	JSONObject outfit;
	{
		int i = 0;
		for (auto s : { "top", "bottom", "hat", "glasses", "mask", "shoes", "bag" })
		{
			outfit[s] = _clothesItems[i] ? new JSONValue(_clothesItems[i]->FullID()) : new JSONValue();
			i++;
		}
	}
	target["outfit"] = new JSONValue(outfit);
}

void Player::Deserialize(JSONObject& source)
{
	Name = source["name"]->AsString();
	Bells = source["bells"]->AsInteger();

	auto& colors = source["colors"]->AsObject();
	SkinTone = GetJSONColor(colors.at("skin"));
	EyeColor = GetJSONColor(colors.at("eyes"));
	CheekColor = GetJSONColor(colors.at("cheek"));
	HairColor = GetJSONColor(colors.at("hair"));

	auto& style = source["style"]->AsObject();
	Gender = StringToEnum<::Gender>(style.at("gender")->AsString(), { "boy", "girl" });
	eyeStyle = style.at("eyes")->AsInteger();
	mouthStyle = style.at("mouth")->AsInteger();
	cheeksStyle = style.at("cheeks")->AsInteger();
	noseStyle = style.at("nose")->AsInteger();
	hairStyle = style.at("hair")->AsInteger();

	auto items = source["items"]->AsArray();
	OnHand.fill(nullptr);
	int j = 0;
	for (const auto& i : items)
	{
		if (i->IsString())
			OnHand[j] = std::make_shared<InventoryItem>(i->AsString());
		j++;
		if (j >= MaxOnHand)
			break;
	}

	auto storage = source["storage"]->AsArray();
	Storage.clear();
	for (const auto& i : storage)
	{
		Storage.push_back(std::make_shared<InventoryItem>(i->AsString()));
		j++;
		if (j >= MaxStorage)
			break;
	}

	auto& outfit = source["outfit"]->AsObject();
	{
		{
			int i = 0;
			for (const auto& s : { "top", "bottom", "hat", "glasses", "mask", "shoes", "bag" })
			{
				auto t = outfit.at(s);
				_clothesItems[i] = t->IsString() ? std::make_shared<InventoryItem>(t->AsString()) : nullptr;
				i++;
			}
		}
	}
}

Player thePlayer;
