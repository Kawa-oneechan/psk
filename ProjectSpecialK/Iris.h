#pragma once

#include "SpecialK.h"

class Iris : public Tickable
{
private:
	float time;
	Shader* shader;
	enum class State
	{
		Idle,
		In,
		Out,
	} state{ State::Idle };

public:
	Iris();
	~Iris();
	void Tick(float dt);
	void Draw(float dt);
	void In();
	void Out();
	bool Done();
};

extern Iris* iris;
