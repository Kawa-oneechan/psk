#pragma once

#include "engine/Tickable.h"

class Iris : public Tickable
{
private:
	float time;
	enum class State
	{
		Idle,
		In,
		Out,
	} state{ State::Idle };

public:
	Iris();
	bool Tick(float dt) override;
	void Draw(float dt) override;
	void In();
	void Out();
	bool Done();
};
