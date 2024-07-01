#pragma once

class Species : public NameableThing
{
public:
	std::string EnName[2];
	bool ModeledMuzzle;

	Species(JSONObject& value, const std::string& filename = "");
	const std::string Name();
};
typedef std::shared_ptr<Species> SpeciesP;
