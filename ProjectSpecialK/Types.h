#pragma once

#include <map>
#include <memory>
#include <sol.hpp>
#include "engine/JsonUtils.h"
#include "engine/Texture.h"

enum class LoadSpawnChoice
{
	FrontDoor,
	MainRoom,
	LastBed,
	InPlace,
};

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;
	extern std::shared_ptr<Texture> controls;

	extern jsonValue json;
	extern jsonValue settings;
	extern std::string initFile;
};

extern Texture* whiteRect;

extern sol::state Sol;
