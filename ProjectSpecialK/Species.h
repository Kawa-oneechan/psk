#pragma once

#include "engine/Model.h"
#include "NameableThing.h"

class Species : public NameableThing
{
private:
	ModelP _model;

public:
	std::string EnNames[2];
	bool ModeledMuzzle;
	std::string FilterAs;

	explicit Species(jsonObject& value, const std::string& filename = "");
	void LoadModel();
	ModelP Model();
};

using SpeciesP = std::shared_ptr<Species>;
