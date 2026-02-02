#pragma once
#include "engine/Model.h"
#include "engine/Tickable.h"
#include "Person.h"
#include "Animator.h"
#include "Species.h"
#include "Traits.h"
#include "Item.h"
#include "Types.h"
#include <sol.hpp>

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

//TODO: split this into its own files
class ScriptRunner : public Tickable
{
public:
	std::shared_ptr<sol::coroutine> currentCoro;
	sol::state myState;
	ScriptRunner(const std::string& entryPoint, const std::string& script, bool* mutex);
	~ScriptRunner() override;
	bool Runnable() const { return currentCoro->runnable(); }
	void Call();
	sol::call_status Status() { return currentCoro->status(); }
};
using ScriptRunnerP = std::shared_ptr<ScriptRunner>;


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
	bool _hasCapViz{ true };

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

	explicit Villager(jsonObject& value, const std::string& filename = "");
	std::string SpeciesName();
	void LoadModel();
	ModelP Model();
	std::string Birthday();
	bool IsSpecial() { return _isSpecial; }
	std::string Catchphrase();
	std::string Catchphrase(const std::string& newPhrase);
	std::string Nickname();
	std::string Nickname(const std::string& newNickname);

	void Draw(float dt) override;
	bool Tick(float dt) override;

	void Manifest();
	void Depart();

	int PickSNPCOutfit(int ordinal) { return PickSNPCOutfit(sol::make_object(Sol, ordinal)); }
	int PickSNPCOutfit(const std::string& id) { return PickSNPCOutfit(sol::make_object(Sol, id)); }
	int PickSNPCOutfit(sol::object a);

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

	bool Mutex{ false };
	ScriptRunnerP scriptRunner;
	void TestScript();
};

using VillagerP = std::shared_ptr<Villager>;
