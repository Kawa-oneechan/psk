#pragma once

typedef enum
{
	Boy, Girl, BEnby, GEnby
} Gender;

//PLACEHOLDER
class Model
{};

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
	const Model* _model;
	const Species* _species;
	bool _customModel;
	bool _isSpecial;
	std::string _customCatchphrase;
	std::string _customNickname;
	Gender gender;
	unsigned char _birthday[2];
	unsigned char _flags[255];
	unsigned char _memories[255];

	static const int _maxFurnitureItems = 8 * 4;
	static const int _maxOutfits = 8 * 3;

	void DeleteAllThings();

public:
	std::string RefSpecies;
	std::string RefCatchphrase;
	std::string NameTag[2];

	const Personality* personality;
	int personalitySubtype;
	const Hobby* hobby;

	std::string umbrellaID;
	std::string photoID;
	std::string portraitID;
	std::string defaultOutfitID;
	std::string rainCoatID;
	std::string rainHatID;

	InventoryItem* HeldTool;
	InventoryItem* Hat;
	InventoryItem* Glasses;
	InventoryItem* Mask;
	InventoryItem* Outfit;

	std::vector<InventoryItem*> Items;
	std::vector<InventoryItem*> Outfits;

	Villager(JSONObject& value);
	const std::string Name();
	const std::string Species();
	void LoadModel();
	const Model* Model();
	const std::string Birthday();
	bool IsSpecial() { return _isSpecial; }
	const std::string Catchphrase();
	std::string Catchphrase(std::string& newPhrase);
	const std::string Nickname();
	std::string Nickname(std::string& newNickname);

	void Manifest();
	void Depart();

	void PickOutfit();

	bool Villager::GiveItem(InventoryItem* item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);
};
