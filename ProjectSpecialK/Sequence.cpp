#include "Sequence.h"

Sequence::Sequence(std::initializer_list<TickableP> parts)
{
	for (auto i : parts)
		ChildTickables.emplace_back(i);
	cursor = 0;
	waiting = parts.size() > 0;
}

bool Sequence::Tick(float dt)
{
	if (ChildTickables.size() == 0)
		return true;
	if (cursor >= ChildTickables.size())
		return true;

	auto ret = false;
	if (waiting)
	{
		auto now = ChildTickables[cursor];
		now->Mutex = &waiting;
		ret = now->Tick(dt);
	}
	//No longer waiting?
	if (!waiting)
	{
		cursor++;
		if (cursor >= ChildTickables.size())
		{
			if (Mutex != nullptr)
				*Mutex = false;
		}
	}
	return ret;
}

void Sequence::Draw(float dt)
{
	if (ChildTickables.size() == 0)
		return;
	if (cursor >= ChildTickables.size())
		return;

	if (waiting)
	{
		auto now = ChildTickables[cursor];
		now->Mutex = &waiting;
		now->Draw(dt);
	}
}

bool FuncAsTickable::Tick(float dt)
{
	dt;
	if (wrapped != nullptr)
		wrapped();
	if (Mutex != nullptr)
		*Mutex = false;
	return true;
}
