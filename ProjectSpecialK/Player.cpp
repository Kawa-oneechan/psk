#include "SpecialK.h"
#include "InputsMap.h"

extern bool useOrthographic;

void Player::LoadModel()
{
	if (!_model)
	{
		_model = std::make_shared<::Model>("player/model.fbx");
		_hairModel = std::make_shared<::Model>(fmt::format("player/hair/{}/model.fbx", 0));
	}

	_model->SetVisibility(fmt::format("Nose{:02}__mNose", noseStyle), true);

	_shoesModel = nullptr;
	_onePieceModel = nullptr;
	_bottomsModel = nullptr;
	_topsModel = nullptr;

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

	/*
	Clothing texture order:
			alb	nml	mix	opc
	top/1p	0	1	2	3
	bottom	4	5	6	7
	shoes	8	9	10	11
	socks	12	13	14	15
	...
	*/

	if (!_onePieceModel && OnePiece)
	{
		_onePieceModel = std::make_shared<::Model>(fmt::format("player/outfits/{}.fbx", OnePiece->PlayerModel()));

		ClothingTextures[0] = new TextureArray(fmt::format("{}/albedo*.png", OnePiece->Path));
		ClothingTextures[1] = new TextureArray(fmt::format("{}/normal.png", OnePiece->Path));
		ClothingTextures[2] = new TextureArray(fmt::format("{}/mix.png", OnePiece->Path));
		ClothingTextures[3] = new TextureArray(fmt::format("{}/opacity.png", OnePiece->Path));
	}
	else if (!_topsModel && Tops)
	{
		_topsModel = std::make_shared<::Model>(fmt::format("player/outfits/{}.fbx", Tops->PlayerModel()));

		ClothingTextures[0] = new TextureArray(fmt::format("{}/albedo*.png", Tops->Path));
		ClothingTextures[1] = new TextureArray(fmt::format("{}/normal.png", Tops->Path));
		ClothingTextures[2] = new TextureArray(fmt::format("{}/mix.png", Tops->Path));
		ClothingTextures[3] = new TextureArray(fmt::format("{}/opacity.png", Tops->Path));
	}

	if (!_bottomsModel && Bottoms)
	{
		_bottomsModel = std::make_shared<::Model>(fmt::format("player/outfits/{}.fbx", Bottoms->PlayerModel()));

		ClothingTextures[4] = new TextureArray(fmt::format("{}/albedo*.png", Bottoms->Path));
		ClothingTextures[5] = new TextureArray(fmt::format("{}/normal.png", Bottoms->Path));
		ClothingTextures[6] = new TextureArray(fmt::format("{}/mix.png", Bottoms->Path));
		ClothingTextures[7] = new TextureArray(fmt::format("{}/opacity.png", Bottoms->Path));
	}


	if (!Socks)
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

ModelP Player::Model(int slot)
{
	switch (slot)
	{
	case 0: return _model;
	case 1: return _hairModel;
	case 2: return _topsModel;
	case 3: return _bottomsModel;
	case 4: return _onePieceModel;
	case 5: return _hatModel;
	case 6: return _glassesModel;
	case 7: return _maskModel;
	case 8: return _shoesModel;
	case 9: return _bagModel;
	default: return nullptr;
	}
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

void Player::Draw(float)
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

	if (_shoesModel && Shoes)
	{
		_shoesModel->SetLayer(Shoes->Variant());
		_shoesModel->Draw(Position, Facing);
	}

	if (_onePieceModel && OnePiece)
	{
		std::copy(&ClothingTextures[0], &ClothingTextures[4], _onePieceModel->GetMesh("_mTops").Textures);
		_onePieceModel->SetLayer(OnePiece->Variant());
		_onePieceModel->Draw(Position, Facing);
	}
	else
	{
		if (_bottomsModel && Bottoms)
		{
			std::copy(&ClothingTextures[4], &ClothingTextures[8], _bottomsModel->GetMesh("_mBottoms").Textures);
			_bottomsModel->SetLayer(Bottoms->Variant());
			_bottomsModel->Draw(Position, Facing);
		}
		if (_topsModel && Tops)
		{
			std::copy(&ClothingTextures[0], &ClothingTextures[4], _topsModel->GetMesh("_mTops").Textures);
			_topsModel->SetLayer(Tops->Variant());
			_topsModel->Draw(Position, Facing);
		}
	}

	//TODO: handle hats, glasses, masks, and bags.
	//Note that certain "glasses" may cover the whole face and disable "masks".
	//Or is that vice versa?
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

	//TODO: reflect bone transforms to clothes.
	//_model->CopyBoneTransforms(_topsModel);

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
	auto tOrP = Tops ? Tops : (OnePiece ? OnePiece : nullptr);
	outfit["topOrOnePiece"] = tOrP ? new JSONValue(tOrP->FullID()) : new JSONValue();
	outfit["bottom"] = Bottoms ? new JSONValue(Bottoms->FullID()) : new JSONValue();
	outfit["shoes"] = Shoes ? new JSONValue(Shoes->FullID()) : new JSONValue();
	outfit["hat"] = Hat ? new JSONValue(Hat->FullID()) : new JSONValue();
	outfit["glasses"] = Glasses ? new JSONValue(Glasses->FullID()) : new JSONValue();
	outfit["mask"] = Mask ? new JSONValue(Mask->FullID()) : new JSONValue();
	outfit["bag"] = Bag ? new JSONValue(Bag->FullID()) : new JSONValue();
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
		auto topOrOnepiece = outfit.at("topOrOnePiece");
		auto bottom = outfit.at("bottom");
		auto shoes = outfit.at("shoes");
		//auto socks = outfit.at("socks");
		auto hat = outfit.at("hat");
		auto glasses = outfit.at("glasses");
		auto mask = outfit.at("mask");
		auto bag = outfit.at("bag");

		Tops = nullptr;
		Bottoms = nullptr;
		OnePiece = nullptr;
		Shoes = nullptr;
		Hat = nullptr;
		Glasses = nullptr;
		Mask = nullptr;
		Bag = nullptr;

		if (topOrOnepiece->IsString())
		{
			auto item = std::make_shared<InventoryItem>(topOrOnepiece->AsString());
			if (item->Wrapped()->ClothingKind == Item::ClothingKind::Tops)
				Tops = item;
			else
				OnePiece = item;
		}
		if (bottom->IsString())
			Bottoms = std::make_shared<InventoryItem>(bottom->AsString());
		if (shoes->IsString())
			Shoes = std::make_shared<InventoryItem>(shoes->AsString());
		if (hat->IsString())
			Hat = std::make_shared<InventoryItem>(hat->AsString());
		if (glasses->IsString())
			Glasses = std::make_shared<InventoryItem>(glasses->AsString());
		if (mask->IsString())
			Mask = std::make_shared<InventoryItem>(mask->AsString());
		if (bag->IsString())
			Bag = std::make_shared<InventoryItem>(bag->AsString());
	}
}


Player thePlayer;
