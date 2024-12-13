#pragma once

#include "SpecialK.h"

class TitleScreen : public Tickable
{
private:
	enum class State
	{
		Init, FadeIn, Wait, FadeOut
	} state{ State::Init };

public:
	TitleScreen();
	~TitleScreen();
	
	void Tick(float dt);
	void Draw(float dt);
};
