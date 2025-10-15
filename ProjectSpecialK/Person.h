#pragma once
#include "engine/Model.h"
#include "engine/Tickable.h"
#include "Animator.h"
#include "Item.h"
#include <sol.hpp>

enum class Gender
{
	Boy, Girl, BEnby, GEnby
};


enum class ClothingSlot
{
	Top, Bottom, Hat, Glasses, Mask, Shoes, Bag, Wetsuit, Socks,
	Dress = ClothingSlot::Top,
	TopFace = ClothingSlot::Glasses,
	BottomFace = ClothingSlot::Mask,
};
#define NumClothes (int)ClothingSlot::Socks

class Person
{
protected:
	ModelP _model;
	std::array<ModelP, 10> _clothesModels;
	std::array<InventoryItemP, 10> _clothesItems;
	std::array<TextureArray*, 24> Textures;
	std::array<TextureArray*, 32> ClothingTextures;

	std::unique_ptr<Animator> animator;
	unsigned int _birthday[2]{ 26, 6 };

public:
	glm::vec3 Position{ 0 };
	float Facing{ 0 };
	int face{ 0 }, mouth{ 0 };

	Gender Gender{ Gender::BEnby };

	void Turn(float facing, float dt);
	bool Move(float facing, float dt);

	void SetFace(int face);
	void SetMouth(int mouth);
	virtual bool Tick(float) = 0;
	virtual void Draw(float);

	Animator* Animator() { return animator.get(); };
};

using PersonP = std::shared_ptr<Person>;
