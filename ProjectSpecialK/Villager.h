#pragma once
#include "Model.h"

enum class Gender
{
	Boy, Girl, BEnby, GEnby
};

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

class Villager : public NameableThing, Tickable
{
private:
	ModelP _model, _clothingModel, _accessoryModel;

	SpeciesP _species{ nullptr };
	bool _customModel{ false };
	bool _customMuzzle{ false };
	bool _customAccessory{ false };
	bool _accessoryFixed{ false };
	bool _isSpecial{ false };
	Gender gender{ Gender::BEnby };
	unsigned int _birthday[2]{ 1, 1 };

	static const int _maxFurnitureItems = 8 * 4;
	static const int _maxClothes = 8 * 3;

	std::array<Texture*, 20> Textures;
	std::array<Texture*, 4> ClothingTextures;

#ifndef DEBUG
	int face{ 0 }, mouth{ 0 };
#endif

	VillagerMemoryP memory;

	void DeleteAllThings();

public:
#ifdef DEBUG
	int face{ 0 }, mouth{ 0 };
	void ReloadTextures();
#endif

	glm::vec3 Position { 0 };
	float Facing{ 0 };

	std::string RefSpecies;
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

	InventoryItemP HeldTool{ nullptr };
	InventoryItemP Hat{ nullptr };
	InventoryItemP Glasses{ nullptr };
	InventoryItemP Mask{ nullptr };
	InventoryItemP Clothing{ nullptr };

	Texture* Icon{ nullptr };

	const bool IsManifest() const { return !(!memory); }

	Villager(JSONObject& value, const std::string& filename = "");
	std::string Name();
	std::string Species();
	void LoadModel();
	ModelP Model();
	std::string Birthday();
	bool IsSpecial() { return _isSpecial; }
	std::string Catchphrase();
	std::string Catchphrase(const std::string& newPhrase);
	std::string Nickname();
	std::string Nickname(const std::string& newNickname);

	void SetFace(int face);
	void SetMouth(int mouth);

	void Draw(double dt);

	void Manifest();
	void Depart();

	void PickClothing();

	bool Villager::GiveItem(InventoryItemP item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);

	//kill copies
	Villager(const Villager&) = delete;
	Villager& operator = (const Villager&) = delete;
	//move instead
	Villager(Villager&&) = default;
	Villager& operator = (Villager&&) = default;
};

using VillagerP = std::shared_ptr<Villager>;
