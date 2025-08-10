#pragma once
#include "engine/Model.h"
#include "Animator.h"
#include "Species.h"
#include "Traits.h"

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

class VillagerMemory
{
public:
	std::string _customCatchphrase;
	std::string _customNickname;

	unsigned char _flags[255]{ 0 };
	unsigned char _memories[255]{ 0 };

	std::vector<InventoryItemP> Items;
	std::vector<InventoryItemP> Clothing;
};

using VillagerMemoryP = std::shared_ptr<VillagerMemory>;

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


class Villager : public NameableThing, public Person
{
private:
	//ModelP _model, _clothingModel, _accessoryModel;
	ModelP _accessoryModel;

	SpeciesP _species{ nullptr };
	bool _customModel{ false };
	bool _customMuzzle{ false };
	bool _customAccessory{ false };
	bool _accessoryFixed{ false };
	bool _isSpecial{ false };

	enum class AccessoryType
	{
		None, Body, Cap, Glass, GlassAlpha, BodyCap
	};
	AccessoryType _accessoryType{ AccessoryType::None };

	static const int _maxFurnitureItems = 8 * 4;
	static const int _maxClothes = 8 * 3;
	
	VillagerMemoryP memory;

	void DeleteAllThings();

public:
#ifdef DEBUG
	void ReloadTextures();
#endif

	std::string RefCatchphrase;
	glm::vec4 NameTag[2]{};

	PersonalityP personality{ nullptr };
	int personalitySubtype{ 0 };
	HobbyP hobby{ nullptr };

	std::string umbrellaID;
	std::string photoID;
	std::string portraitID;
	std::string defaultClothingID;
	std::string rainCoatID;
	std::string rainHatID;

	Texture* Icon{ nullptr };

	const bool IsManifest() const { return !(!memory); }
	Species* Species();

	Villager(jsonObject& value, const std::string& filename = "");
	std::string Name();
	std::string SpeciesName();
	void LoadModel();
	ModelP Model();
	std::string Birthday();
	bool IsSpecial() { return _isSpecial; }
	std::string Catchphrase();
	std::string Catchphrase(const std::string& newPhrase);
	std::string Nickname();
	std::string Nickname(const std::string& newNickname);

	void Draw(float dt);
	bool Tick(float dt);

	void Manifest();
	void Depart();

	void PickClothing();

	bool GiveItem(InventoryItemP item);

	void Serialize(jsonObject& target);
	void Deserialize(jsonObject& source);

	//kill copies
	Villager(const Villager&) = delete;
	Villager& operator = (const Villager&) = delete;
	//move instead
	Villager(Villager&&) = default;
	Villager& operator = (Villager&&) = default;

	InventoryItem* Clothing() { return _clothesItems[0].get(); };
};

using VillagerP = std::shared_ptr<Villager>;
