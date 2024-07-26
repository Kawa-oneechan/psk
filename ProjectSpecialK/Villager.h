#pragma once

#include "Model.h"

enum class Gender
{
	Boy, Girl, BEnby, GEnby
};

/*
House stuff, exterior:
* Flag
* Level
* Status
* Walls
* Roof
* Door
* Owner
* Player?
*/

class Villager : public NameableThing, Tickable
{
private:
	ModelP _model;
	SpeciesP _species{ nullptr };
	bool _customModel{ false };
	bool _isSpecial{ false };
	std::string _customCatchphrase;
	std::string _customNickname;
	Gender gender{ Gender::BEnby };
	unsigned int _birthday[2]{ 1, 1 };
	unsigned char _flags[255]{ 0 };
	unsigned char _memories[255]{ 0 };

	static const int _maxFurnitureItems = 8 * 4;
	static const int _maxOutfits = 8 * 3;

	void DeleteAllThings();

public:
	std::string RefSpecies;
	std::string RefCatchphrase;
	glm::vec4 NameTag[2]{};

	PersonalityP personality{ nullptr };
	int personalitySubtype{ 0 };
	HobbyP hobby{ nullptr };

	std::string umbrellaID;
	std::string photoID;
	std::string portraitID;
	std::string defaultOutfitID;
	std::string rainCoatID;
	std::string rainHatID;

	InventoryItemP HeldTool{ nullptr };
	InventoryItemP Hat{ nullptr };
	InventoryItemP Glasses{ nullptr };
	InventoryItemP Mask{ nullptr };
	InventoryItemP Outfit{ nullptr };

	std::vector<InventoryItemP> Items;
	std::vector<InventoryItemP> Outfits;

	Villager(JSONObject& value, const std::string& filename = "");
	std::string Name();
	std::string Species();
	void LoadModel();
	ModelP Model();
	std::string Birthday();
	bool IsSpecial() { return _isSpecial; }
	std::string Catchphrase();
	std::string Catchphrase(std::string& newPhrase);
	std::string Nickname();
	std::string Nickname(std::string& newNickname);

	void Manifest();
	void Depart();

	void PickOutfit();

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
