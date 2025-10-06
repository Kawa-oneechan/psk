#pragma once

#include "DoomMenu.h"

class OptionsMenu : public DoomMenu
{
protected:
	void Build();
	void Translate();

	DoomMenuPage options, content, keybinds, volume, species;

	std::vector<Texture*> speciesPreviews;
	std::string speciesText;

public:
	OptionsMenu();
	bool Scancode(unsigned int scancode) override;
};
