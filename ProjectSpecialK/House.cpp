#include "engine/Console.h"
#include "House.h"
#include "Town.h"
#include "Database.h"
#include "Player.h"

VillagerHouse::Room::Room(VillagerHouse* parent, size_t roomNum)
{
	parentHouse = parent;
	parentRoomNum = roomNum;

	Width = 64;
	Height = 64;
	Terrain = std::make_unique<MapTile[]>(Width * Height);
	//if (roomNum == 0)
	Exits[0] = ToOutside;

	//TODO: generate invisible terrain from room size.
}

float VillagerHouse::Room::GetHeight(const glm::vec3& pos)
{
	//TODO: trick out the stairs.
	pos;
	return 0.0f;
}

float VillagerHouse::Room::GetHeight(int x, int y)
{
	return GetHeight(glm::vec3(x, 100, y));
}

void VillagerHouse::Room::Draw(float dt)
{
	dt;
}

bool VillagerHouse::Room::Tick(float dt)
{
	dt;
	return true;
}

void VillagerHouse::SaveObjects(jsonValue& json)
{
	//Adapted from Town
	//TODO: correct format
	auto objects = json5pp::array({});
	for (int r = 0; r < Rooms.size(); r++)
	{
		for (const auto& i : Rooms[r].Objects)
		{
			auto item = json5pp::object({});
			item.as_object()["room"] = r;
			item.as_object()["id"] = i.Item->FullID();
			item.as_object()["position"] = GetJSONVec(i.Position / 10.0f, true);
			if (i.State != 0)
			{
				item.as_object()["state"] = i.State;
			}
			if (i.Dropped)
			{
				item.as_object()["dropped"] = true;
			}
			else
			{
				if (i.Fixed)
					item.as_object()["fixed"] = true;
				if (i.Rotation != 0)
					item.as_object()["facing"] = i.Rotation;
				if (i.Layer != ItemLayer::Ground)
					item.as_object()["layer"] = (int)i.Layer;
			}
			objects.as_array().push_back(item);
		}
	}
	json.as_object()["objects"] = std::move(objects);
}

void VillagerHouse::LoadObjects(jsonValue& json, int roomNum)
{
	//Adapted from Town
	if (Rooms.empty())
	{
		conprint(4, "VillagerHouse::LoadObjects: must have Rooms to load to first.");
		return;
	}
	if (roomNum >= Rooms.size())
	{
		conprint(4, "VillagerHouse::LoadObjects: tried to load out-of-range room objects.");
		return;
	}

	auto doc = json.as_object();
	auto objects = doc["objects"].is_array() ? doc["objects"].as_array() : json5pp::array({}).as_array();

	for (auto& _i : objects)
	{
		auto i = _i.as_object();
		MapItem item;
		item.Item = std::make_shared<InventoryItem>(i["id"].as_string());
		item.Position = GetJSONVec2(i["position"]) * 10.0f;
		item.State = i["state"].is_number() ? (int)i["state"].as_number() : 0;
		item.Dropped = i["dropped"].is_boolean() ? i["dropped"].as_boolean() : false;
		if (item.Dropped)
		{
			item.Fixed = false;
			item.Layer = ItemLayer::Ground;
			item.Rotation = 0;
		}
		else
		{
			item.Fixed = i["fixed"].is_boolean() ? i["fixed"].as_boolean() : false;
			item.Layer = (ItemLayer)(i["layer"].is_integer() ? i["layer"].as_integer() : 0);
			item.Rotation = i["facing"].is_integer() ? i["facing"].as_integer() : 0;
		}
		Rooms[roomNum].Objects.push_back(item);
	}

	//TODO: go through the list of objects later on to correct wrong layers.
	//Items on the table layer should have a supporter on the ground layer.
}

void VillagerHouse::Load()
{
	if (Owner == Orphaned)
	{
		conprint(4, "VillagerHouse::Load: cannot load orphaned house.");
		return;
	}

	jsonValue ownerJson;

	if (IsPlayerOwned)
	{
		//Remember to properly identify which player later on.
		auto savedHouseFile = fmt::format("homes/{}", thePlayer.ID);
		ownerJson = VFS::ReadSaveJSON(savedHouseFile);
		//Catch if this worked or not.
	}
	else
	{
		VillagerP owner = nullptr;
		owner = Database::Find<Villager>(Owner, villagers);
		auto savedHouseFile = fmt::format("homes/{}.json", owner);

		ownerJson = VFS::ReadSaveJSON(savedHouseFile);
		//Catch if this worked or not.


		//if not...
		{
			auto ownerFile = VFS::Enumerate(fmt::format("{}/*.json", owner->Path))[0];
			auto temp = VFS::ReadJSON(ownerFile);
			auto houseData = temp.as_object()["house"];
			ownerJson = houseData.as_object()["rooms"];
		}
	}

	for (auto& room : ownerJson.as_array())
	{
		int roomNum = (int)Rooms.size();
		Rooms.emplace_back(this, roomNum);
		LoadObjects(room, roomNum);
	}
}

VillagerHouse::VillagerHouse(int owner)
{
	if (owner == -1)
	{
		Owner = thePlayer.Hash;
		IsPlayerOwned = true;
	}
	else if (owner >= town->Villagers.size())
	{
		conprint(4, "VillagerHouse: owner {} out range ({}).", owner, town->Villagers.size());
		Owner = Orphaned;
		IsPlayerOwned = false;
		return;
	}
	else
	{
		Owner = town->Villagers[owner]->Hash;
		IsPlayerOwned = false;
	}

	Load();
}
