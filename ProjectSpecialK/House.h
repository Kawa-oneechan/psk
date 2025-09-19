#pragma once

#include "Map.h"

class VillagerHouse
{
public:
	//Maximum amount of rooms a VillagerHouse can have.
	static constexpr int MaxRooms = 16;
	//Special exit value for when there's no room that way.
	static constexpr int NoRoom = -1;
	//Special exit value for when the exit is that way.
	static constexpr int ToOutside = -2;

	static constexpr hash Orphaned = (hash)-1;
	static constexpr hash PlayerHouse = (hash)-2;

	class Room : public Map
	{
		VillagerHouse* parentHouse{ nullptr };
		size_t parentRoomNum{ 0 };

	public:
		//Exit destinations in the order south, east, north, west (counterclockwise),
		//then south-up and south-down. These map into VillagerHouse::Rooms[].
		int Exits[6] = { NoRoom, NoRoom, NoRoom, NoRoom, NoRoom, NoRoom };

		int SizeClass = { 0 };

		hash Walls{ 0 }, Floor{ 0 }, Music{ 0 };
		std::vector<MapItem> Objects;

		Room(VillagerHouse* parent, size_t roomNum);

		float GetHeight(const glm::vec3& pos);
		float GetHeight(int x, int y);

		void Draw(float);
		bool Tick(float);
	};

	//Rooms[0] is the MAIN room, which should always have Exits[0] == ToOutside.
	std::vector<Room> Rooms;

	hash Owner;
	bool IsPlayerOwned;

	VillagerHouse(int owner);

	void SaveObjects(jsonValue& json);
	void LoadObjects(jsonValue& json);

	void Load();
	void Save();
};
