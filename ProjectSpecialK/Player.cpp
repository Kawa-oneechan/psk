#include "SpecialK.h"
#include "InputsMap.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/rotate_vector.hpp"

void Player::LoadModel()
{
	if (!_model)
	{
		_model = std::make_shared<::Model>("player/model.fbx");
	}

	_model->SetVisibility("Nose01__mNose", true);

	_hairModel = nullptr;
	_shoesModel = nullptr;
	_onePieceModel = nullptr;
	_bottomsModel = nullptr;
	_topsModel = nullptr;

	if (Textures[0] == nullptr)
	{
		Textures[0] = new TextureArray("player/eyes/0/eye*_alb.png");
		Textures[1] = new TextureArray("player/eyes/0/eye*_nrm.png");
		Textures[2] = new TextureArray("player/eyes/0/eye*_mix.png");

		Textures[3] = new TextureArray("player/mouth/0/mouth*_alb.png");
		Textures[4] = new TextureArray("player/mouth/0/mouth*_nrm.png");
		Textures[5] = new TextureArray("player/mouth/0/mouth*_mix.png");

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

void Player::SetFace(int index)
{
	face = clamp(index, 0, 15);
}
void Player::SetMouth(int index)
{
	mouth = clamp(index, 0, 8);
}

void Player::Turn(float facing)
{
	auto m = Facing;
	if (m < 0) m += 360.0f;

	auto cw = facing - m;
	if (cw < 0.0) cw += 360.0f;
	auto ccw = m - facing;
	if (ccw < 0.0) ccw += 360.0f;

	auto t = (ccw < cw) ? -glm::min(10.0f, ccw) : glm::min(10.0f, cw);

	auto f = m + t;
	if (f < 0) f += 360.0f;

	Facing = glm::mod(f, 360.0f);
}

bool Player::Move(float facing)
{
	Turn(facing);

	const auto movement = glm::rotate(glm::vec2(0, 0.25f), glm::radians(Facing));

	//TODO: determine collisions.
	Position.x -= movement.x;
	Position.z += movement.y;
	return true;
}

void Player::Draw(float)
{
	if (!_model)
		LoadModel();

	//TODO: Model::SetTexture method, yo!
	std::copy(&Textures[0], &Textures[3], _model->GetMesh("_mEye").Textures);
	std::copy(&Textures[3], &Textures[6], _model->GetMesh("_mMouth").Textures);
	_model->GetMesh("_mCheek").Textures[0] = Textures[6];

	_model->SetLayer("_mCheek", cheeks);
	_model->SetLayer("_mEye", face);
	_model->SetLayer("_mMouth", mouth);

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
			_bottomsModel->SetLayer(Bottoms->Variant());
			_bottomsModel->Draw(Position, Facing);
		}
		if (_topsModel && Tops)
		{
			_topsModel->SetLayer(Tops->Variant());
			_topsModel->Draw(Position, Facing);
		}
	}

	//TODO: handle hats, glasses, masks, and bags.
	//Note that certain "glasses" may cover the whole face and disable "masks".
	//Or is that vice versa?
}

void Player::Tick(float dt)
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
		Move(facing + MainCamera.Angles().z);
	}
}

Player thePlayer;
