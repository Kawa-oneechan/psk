#pragma once

class Species : public NameableThing
{
public:
	std::string EnName[2];
	bool ModeledMuzzle;
	std::string FilterAs;

	Species(JSONObject& value, const std::string& filename = "");
	const std::string Name();
};

using SpeciesP = std::shared_ptr<Species>;
