#pragma once

#include <functional>
#include "engine/Tickable.h"

class Sequence : public Tickable
{
private:
	int cursor;
	bool waiting;

public:
	Sequence(std::initializer_list<TickableP> tickables);
	bool Tick(float dt) override;
	void Draw(float dt) override;
};

class FuncAsTickable : public Tickable
{
private:
	std::function<void()> wrapped;

public:
	FuncAsTickable(std::function<void()> function) : wrapped(function) {}
	bool Tick(float dt) override;
};
