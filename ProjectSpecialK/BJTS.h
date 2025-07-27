#pragma once
#include <map>
#include "engine/Types.h"

typedef void(*BJTSFunc)(std::string& data, BJTSParams);

//BJTS functions that actually change the string content.
extern const std::map<std::string, BJTSFunc> bjtsPhase1;
//BJTS functions loaded from Lua scripts.
extern std::map<std::string, std::string> bjtsPhase1X;
