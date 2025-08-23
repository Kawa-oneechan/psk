#pragma once
#include <vector>
#include <memory>

class Tickable
{
public:
	bool* Mutex{ nullptr };
	bool Dead{ false };
	bool Visible{ true };
	bool Enabled{ true };
	std::vector<std::shared_ptr<Tickable>> ChildTickables;

	virtual ~Tickable() {}
	virtual bool Tick(float) { return true; };
	virtual void Draw(float) {};
	virtual bool Character(unsigned int ch)
	{
		for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
		{
			auto t = ChildTickables[i];
			if (!t->Enabled)
				continue;
			if (t->Character(ch))
				return true;
		}
		return false;
	}
	virtual bool Scancode(unsigned int sc)
	{
		for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
		{
			auto t = ChildTickables[i];
			if (!t->Enabled)
				continue;
			if (t->Scancode(sc))
				return true;
		}
		return false;
	}
};

using TickableP = std::shared_ptr<Tickable>;
