#pragma once

#include "Model.h"

class Species : public NameableThing
{
private:
	ModelP _model;

public:
	std::string EnName[2];
	bool ModeledMuzzle;
	std::string FilterAs;

	Species(JSONObject& value, const std::string& filename = "");
	std::string Name();
	void LoadModel();
	ModelP Model();
};

using SpeciesP = std::shared_ptr<Species>;
