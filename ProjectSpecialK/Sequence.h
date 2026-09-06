#pragma once

#include <functional>
#include "engine/Tickable.h"
#include "Scriptable.h"

class Sequence : public Tickable, public Scriptable
{
private:
	int cursor;
	bool waiting;

public:
	Sequence(std::initializer_list<TickableP> parts);
	bool Tick(float dt) override;
	void Draw(float dt) override;
};

class FuncAsTickable : public Tickable, public Scriptable
{
private:
	std::function<void()> wrapped;

public:
	explicit FuncAsTickable(std::function<void()> function) : wrapped(function) {}
	bool Tick(float dt) override;
};
