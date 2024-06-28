#pragma once

enum Gender
{
	Boy, Girl, BEnby, GEnby
};

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
	const Model* _model{ nullptr };
	const Species* _species{ nullptr };
	bool _customModel{ false };
	bool _isSpecial{ false };
	std::string _customCatchphrase{ "" };
	std::string _customNickname{ "" };
	Gender gender{ Gender::BEnby };
	unsigned char _birthday[2]{ 1, 1 };
	unsigned char _flags[255]{ 0 };
	unsigned char _memories[255]{ 0 };

	static const int _maxFurnitureItems = 8 * 4;
	static const int _maxOutfits = 8 * 3;

	void DeleteAllThings();

public:
	std::string RefSpecies{ "" };
	std::string RefCatchphrase{ "" };
	glm::vec4 NameTag[2]{};

	const Personality* personality{ nullptr };
	int personalitySubtype{ 0 };
	const Hobby* hobby{ nullptr };

	std::string umbrellaID{ "" };
	std::string photoID{ "" };
	std::string portraitID{ "" };
	std::string defaultOutfitID{ "" };
	std::string rainCoatID{ "" };
	std::string rainHatID{ "" };

	std::shared_ptr<InventoryItem> HeldTool{ nullptr };
	std::shared_ptr<InventoryItem> Hat{ nullptr };
	std::shared_ptr<InventoryItem> Glasses{ nullptr };
	std::shared_ptr<InventoryItem> Mask{ nullptr };
	std::shared_ptr<InventoryItem> Outfit{ nullptr };

	std::vector<std::shared_ptr<InventoryItem>> Items;
	std::vector<std::shared_ptr<InventoryItem>> Outfits;

	Villager(JSONObject& value, const std::string& filename = "");
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

	bool Villager::GiveItem(std::shared_ptr<InventoryItem> item);

	void Serialize(JSONObject& target);
	void Deserialize(JSONObject& source);

	//kill copies
	Villager(const Villager&) = delete;
	Villager& operator = (const Villager&) = delete;
	//move instead
	Villager(Villager&&) = default;
	Villager& operator = (Villager&&) = default;
};
