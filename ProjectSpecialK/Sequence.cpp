#include "Sequence.h"

Sequence::Sequence(std::initializer_list<TickableP> tickables)
{
	for (auto i : tickables)
		parts.emplace_back(i);
	cursor = 0;
	waiting = parts.size() > 0;
}

bool Sequence::Tick(float dt)
{
	if (parts.size() == 0)
		return true;
	if (cursor >= parts.size())
		return true;

	auto ret = false;
	if (waiting)
	{
		auto now = parts[cursor];
		now->mutex = &waiting;
		ret = now->Tick(dt);
	}
	//No longer waiting?
	if (!waiting)
	{
		cursor++;
		if (cursor >= parts.size())
		{
			if (mutex != nullptr)
				*mutex = false;
		}
	}
	return ret;
}

void Sequence::Draw(float dt)
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

bool FuncAsTickable::Tick(float dt)
{
	dt;
	if (wrapped != nullptr)
		wrapped();
	if (mutex != nullptr)
		*mutex = false;
	return true;
}
