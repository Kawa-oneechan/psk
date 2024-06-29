#include "Sequence.h"

Sequence::Sequence(std::initializer_list<TickableP> tickables)
{
	for (auto i : tickables)
		parts.emplace_back(i);
	cursor = 0;
	waiting = parts.size() > 0;
}

void Sequence::Tick(double dt)
{
	if (parts.size() == 0)
		return;
	if (cursor >= parts.size())
		return;

	if (waiting)
	{
		auto now = parts[cursor];
		now->mutex = &waiting;
		now->Tick(dt);
	}
	//No longer waiting?
	if (!waiting)
	{
		//Is this safe?
		//delete now;

		cursor++;
		//Reached end?
		if (cursor >= parts.size())
		{
			if (mutex != nullptr)
				*mutex = false;
		}
	}
}

void Sequence::Draw(double dt)
{
	if (parts.size() == 0)
		return;
	if (cursor >= parts.size())
		return;

	if (waiting)
	{
		auto now = parts[cursor];
		now->mutex = &waiting;
		now->Draw(dt);
	}
}

void FuncAsTickable::Tick(double dt)
{
	if (wrapped != nullptr)
		wrapped();
	if (mutex != nullptr)
		*mutex = false;
}
