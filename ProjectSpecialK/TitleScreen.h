#pragma once

#include "engine/Tickable.h"
#include "Iris.h"
#include "PanelLayout.h"
#include "OptionsMenu.h"
#include "engine/DropLabel.h"
#include "engine/NineSlicer.h"

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
	std::shared_ptr<NineSlicer> playerPanel;

	float panelPop{ 0 };

	//TODO: make DropLabels Texture2D
	DropLabel* pressStart = nullptr;

public:
	TitleScreen();
	~TitleScreen() override;

	bool Tick(float dt) override;
	void Draw(float dt) override;
};
