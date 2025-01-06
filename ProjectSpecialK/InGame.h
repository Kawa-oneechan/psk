#pragma once

#include "SpecialK.h"

class InGame : public Tickable
{
private:
	enum class State
	{
		Init, FadeIn, Playing
	} state{ State::Init };

public:
	InGame();
	~InGame();

	bool Tick(float dt);
	void Draw(float dt);
};
