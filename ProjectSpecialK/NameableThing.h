#pragma once
#include "engine/Types.h"
#include "engine/JsonUtils.h"

class NameableThing
{
public:
	std::string ID;
	std::string RefName;
	std::string EnName;
	std::string Path; //To locate specific stuff like models, textures, sounds...
	std::string File; //Goes with the path: Path + "/" + File.
	hash Hash{ 0 };
	explicit NameableThing(jsonObject& value, const std::string& filename = "");
	NameableThing() = default;
	const std::string Name() const;

	inline bool operator==(const NameableThing& r) const { return this->ID == r.ID; }
	inline bool operator!= (const NameableThing& r) const { return this->ID != r.ID; }
	inline bool operator== (const std::string& r) const { return this->ID == r; }
	inline bool operator!= (const std::string& r) const { return this->ID != r; }
	inline bool operator== (unsigned int r) const { return this->Hash == r; }
	inline bool operator!= (unsigned int r) const { return this->Hash != r; }
};
