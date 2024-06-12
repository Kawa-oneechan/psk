#pragma once

#include "SpecialK.h"

class Sequence : public Tickable
{
private:
	std::vector<Tickable*> parts;
	int cursor;
	bool waiting;

public:
	Sequence(std::initializer_list<Tickable*> tickables);
	void Tick(double dt);
	void Draw(double dt);
};

class FuncAsTickable : public Tickable
{
private:
	std::function<void()> wrapped;

public:
	FuncAsTickable(std::function<void()> function) : wrapped(function) {}
	void Tick(double dt);
};
