#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

class TitleScreen : public Tickable
{
private:
	PanelLayout* logoAnim;

	enum class State
	{
		Init, FadeIn, Wait, FadeOut
	} state{ State::Init };
	float fade;

public:
	TitleScreen();
	~TitleScreen();
	
	void Tick(float dt);
	void Draw(float dt);
};
