#pragma once

#include "SpecialK.h"
#include "Iris.h"
#include "PanelLayout.h"
#include "DoomMenu.h"

class TitleScreen : public Tickable
{
private:
	enum class State
	{
		Init, FadeIn, Wait, FadeOut
	} state{ State::Init };

	std::shared_ptr<PanelLayout> logoAnim;
	std::shared_ptr<OptionsMenu> optionsMenu;
	std::shared_ptr<Iris> iris;

	std::string psText;
	glm::vec2 psSize;

public:
	TitleScreen();
	
	bool Tick(float dt);
	void Draw(float dt);
};
