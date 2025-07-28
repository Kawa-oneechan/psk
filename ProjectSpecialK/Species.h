#pragma once

#include "engine/Model.h"

class Species : public NameableThing
{
private:
	ModelP _model;

public:
	std::string EnName[2];
	bool ModeledMuzzle;
	std::string FilterAs;

	Species(jsonObject& value, const std::string& filename = "");
	std::string Name();
	void LoadModel();
	ModelP Model();
};

using SpeciesP = std::shared_ptr<Species>;
