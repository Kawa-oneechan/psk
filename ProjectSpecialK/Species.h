#pragma once

class Species
{
public:
	std::string ID;
	std::string RefName;
	std::string EnName[2];
	bool ModeledMuzzle;

	Species(JSONObject& value, const std::string& filename = "");
	const std::string Name();
};
