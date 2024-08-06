#pragma once

#include "support/JSON/JSON.h"

class NameableThing
{
public:
	std::string ID;
	std::string RefName;
	std::string EnName;
	std::string Path; //To locate specific stuff like models, textures, sounds...
	unsigned int Hash;
	NameableThing(JSONObject& value, const std::string& filename = "");
	NameableThing() = default;
	std::string Name();

	inline bool operator== (const NameableThing& r) { return this->ID == r.ID; }
	inline bool operator!= (const NameableThing& r) { return this->ID != r.ID; }
	inline bool operator== (const std::string& r) { return this->ID == r; }
	inline bool operator!= (const std::string& r) { return this->ID != r; }
	inline bool operator== (unsigned int r) { return this->Hash == r; }
	inline bool operator!= (unsigned int r) { return this->Hash != r; }
};
