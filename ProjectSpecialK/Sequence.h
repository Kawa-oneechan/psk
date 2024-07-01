#pragma once

#include "SpecialK.h"

class Sequence : public Tickable
{
private:
	std::vector<TickableP> parts;
	int cursor;
	bool waiting;

public:
	Sequence(std::initializer_list<TickableP> tickables);
	void Tick(float dt);
	void Draw(float dt);
};

class FuncAsTickable : public Tickable
{
private:
	std::function<void()> wrapped;

public:
	FuncAsTickable(std::function<void()> function) : wrapped(function) {}
	void Tick(float dt);
};
