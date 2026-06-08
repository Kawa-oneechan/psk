#pragma once

#include "engine/Tickable.h"

class Iris : public Tickable
{
private:
	float time{ 1.0f }; //assume we're all black, irising IN;
	enum class State
	{
		Idle,
		In,
		Out,
	} state{ State::Idle };

public:
	bool Tick(float dt) override;
	void Draw(float dt) override;
	void In();
	void Out();
	bool Done() const;
};
