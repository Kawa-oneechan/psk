#include "Sequence.h"

Sequence::Sequence(std::initializer_list<TickableP> parts)
{
	for (auto i : parts)
		tickables.emplace_back(i);
	cursor = 0;
	waiting = parts.size() > 0;
}

bool Sequence::Tick(float dt)
{
	if (tickables.size() == 0)
		return true;
	if (cursor >= tickables.size())
		return true;

	auto ret = false;
	if (waiting)
	{
		auto now = tickables[cursor];
		now->mutex = &waiting;
		ret = now->Tick(dt);
	}
	//No longer waiting?
	if (!waiting)
	{
		cursor++;
		if (cursor >= tickables.size())
		{
			if (mutex != nullptr)
				*mutex = false;
		}
	}
	return ret;
}

void Sequence::Draw(float dt)
{
	if (tickables.size() == 0)
		return;
	if (cursor >= tickables.size())
		return;

	if (waiting)
	{
		auto now = tickables[cursor];
		now->mutex = &waiting;
		now->Draw(dt);
	}
}

bool FuncAsTickable::Tick(float dt)
{
	dt;
	if (wrapped != nullptr)
		wrapped();
	if (mutex != nullptr)
		*mutex = false;
	return true;
}
