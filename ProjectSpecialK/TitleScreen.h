#pragma once

#include "engine/Tickable.h"
#include "Iris.h"
#include "PanelLayout.h"
#include "OptionsMenu.h"
#include "engine/DropLabel.h"

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

	//std::string psText;
	//glm::vec2 psSize;
	DropLabel* pressStart = nullptr;

public:
	TitleScreen();
	
	bool Tick(float dt);
	void Draw(float dt);
};
