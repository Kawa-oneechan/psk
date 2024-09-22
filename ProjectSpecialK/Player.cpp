#include "SpecialK.h"

void Player::LoadModel()
{

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

extern Shader* modelShader;
void Player::Draw(double)
{
	if (_model == nullptr)
		LoadModel();

	//TODO: set up and use playerModelShader instead, until it's time to do the clothes.
	modelShader->Use();

	//Body
	_model->Textures[0] = Textures[0]; //-V1004 I know LoadModel doesn't actually *set* _model.
	_model->Textures[1] = Textures[1];
	_model->Textures[2] = Textures[2];
	//Nose
	_model->Textures[4] = Textures[3];
	_model->Textures[5] = Textures[4];
	_model->Textures[6] = Textures[5];
	//Cheek
	_model->Textures[8] = Textures[6];
	_model->Textures[9] = Textures[7];
	_model->Textures[10] = Textures[8];
	//Eyes
	if (!stung)
	{
		_model->Textures[12] = Textures[9];
		_model->Textures[13] = Textures[10];
		_model->Textures[14] = Textures[11];
	}
	else
	{
		_model->Textures[12] = Textures[12];
		_model->Textures[13] = Textures[13];
		_model->Textures[14] = Textures[14];
	}
	//Mouths
	_model->Textures[16] = Textures[15];
	_model->Textures[17] = Textures[16];
	_model->Textures[18] = Textures[17];

	_model->TexArrayLayers[3] = face;
	_model->TexArrayLayers[4] = mouth;

	_model->Draw();

	if (_hairModel)
	{
		_hairModel->Textures[0] = Textures[3];
		_hairModel->Textures[1] = Textures[4];
		_hairModel->Textures[2] = Textures[5];
		_hairModel->Draw();
	}

	if (_shoesModel && Shoes)
	{
		_shoesModel->Textures[0] = ClothingTextures[8];
		_shoesModel->Textures[1] = ClothingTextures[9];
		_shoesModel->Textures[2] = ClothingTextures[10];
		_shoesModel->Textures[3] = ClothingTextures[11];

		_shoesModel->TexArrayLayers[0] = Shoes->Variant();
		_shoesModel->Draw();
	}

	if (_onePieceModel && OnePiece)
	{
		_onePieceModel->Textures[0] = ClothingTextures[0];
		_onePieceModel->Textures[1] = ClothingTextures[1];
		_onePieceModel->Textures[2] = ClothingTextures[2];
		_onePieceModel->Textures[3] = ClothingTextures[3];

		_onePieceModel->TexArrayLayers[0] = OnePiece->Variant();
		_onePieceModel->Draw();
	}
	else
	{
		if (_bottomsModel && Bottoms)
		{
			_bottomsModel->Textures[0] = ClothingTextures[4];
			_bottomsModel->Textures[1] = ClothingTextures[5];
			_bottomsModel->Textures[2] = ClothingTextures[6];
			_bottomsModel->Textures[3] = ClothingTextures[7];

			_bottomsModel->TexArrayLayers[0] = Bottoms->Variant();
			_bottomsModel->Draw();
		}
		if (_topsModel && Tops)
		{
			_topsModel->Textures[0] = ClothingTextures[0];
			_topsModel->Textures[1] = ClothingTextures[1];
			_topsModel->Textures[2] = ClothingTextures[2];
			_topsModel->Textures[3] = ClothingTextures[3];

			_topsModel->TexArrayLayers[0] = Tops->Variant();
			_topsModel->Draw();
		}
	}

	//TODO: handle hats, glasses, masks, and bags.
	//Note that certain "glasses" may cover the whole face and disable "masks".
	//Or is that vice versa?
}

Player thePlayer;
